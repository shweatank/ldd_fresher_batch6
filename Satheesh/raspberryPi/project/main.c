#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/rtc.h>
#include <linux/timekeeping.h>
#include<linux/gpio.h>

#define DEVICE_NAME "MONITORING_SYSTEM"
#define CLASS_NAME "my_class"
/* For Leds */
#define LOW (16+512)
#define MED (20+512)
#define HIGH (21+512)


#define PERI_BASE      0xFE000000UL
#define GPIO_BASE      (PERI_BASE + 0x200000)
#define SPI0_BASE      (PERI_BASE + 0x204000)
#define PWM_BASE       0xFE20C000
#define PWMCLK_BASE    0xFE1010A0

/* PWM Registers */
#define PWMCTL         0x00
#define PWMRNG1        0x10
#define PWMDAT1        0x14
#define PWMCLK_CNTL    0x00
#define PWMCLK_DIV     0x04

/* GPIO Registers */
#define GPFSEL0        0x00
#define GPFSEL1        0x04

/* SPI Registers */
#define SPI_CS         0x00
#define SPI_FIFO       0x04
#define SPI_CLK        0x08

#define SPI_CS_TXD      (1 << 18)
#define SPI_CS_RXD      (1 << 17)
#define SPI_CS_DONE     (1 << 16)
#define SPI_CS_TA       (1 << 7)
#define SPI_CS_CLEAR_RX (1 << 5)
#define SPI_CS_CLEAR_TX (1 << 4)

/* UART Registers */
#define DR      0x00
#define FR      0x18
#define IBRD    0x24
#define FBRD    0x28
#define LCRH    0x2C
#define CR      0x30
#define IMSC    0x38
#define MIS     0x40
#define ICR     0x44

#define FR_RX (1<< 4)
#define FR_TX (1 << 5)

#define MCP3208_CH0 0
struct timespec64 ts;

struct rtc_time tm;

/* Virtual mapped addresses */
static void __iomem *uart_base,*gpio_base,*pwm_base,*pwmclk_base,*spi_base;

/* Buffers */
static char rx_buffer[64],log_buffer[256];
static int rx_index,data_ready;

/* Character device */
static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;

/* Timer + Workqueue + Waitqueue */
static struct timer_list sample_timer;
static struct workqueue_struct *my_wq;
static struct work_struct my_work;
static wait_queue_head_t read_queue;

/* Send one byte through UART */
static void uart_tx(char ch)
{
    while(readl(uart_base + FR) & FR_TX)
        cpu_relax();

    writel(ch, uart_base + DR);
}

/* Send full string through UART */
static void uart_tx_string(char *str)
{
    while(*str)
        uart_tx(*str++);
}

/* UART Interrupt Handler */
static irqreturn_t uart_irq_handler(int irq, void *dev_id)
{
    u32 status = readl(uart_base + MIS);

    /* Check RX interrupt */
    if(status & (1 << 4)) {

        char ch = readl(uart_base + DR);

        /* Store received data */
        if(rx_index < 63) {
            rx_buffer[rx_index++] = ch;
            rx_buffer[rx_index] = '\0';
        }

        pr_info("RX : %c\n", ch);

        /* Clear interrupt */
        writel((1 << 4), uart_base + ICR);
    }

    return IRQ_HANDLED;
}

/* Read SPI register */
static inline u32 spi_read(u32 reg)
{
    return readl(spi_base + reg);
}

/* Write SPI register */
static inline void spi_write(u32 reg,u32 value)
{
    writel(value, spi_base + reg);
}

/* Configure SPI GPIO pins */
static void spi_gpio_init(void)
{
    u32 reg;

    reg = readl(gpio_base + GPFSEL0);

    /* GPIO7 GPIO8 GPIO9 ALT0 */
    reg &= ~((7 << 21) | (7 << 24) | (7 << 27));
    reg |=  ((4 << 21) | (4 << 24) | (4 << 27));

    writel(reg, gpio_base + GPFSEL0);

    reg = readl(gpio_base + GPFSEL1);

    /* GPIO10 GPIO11 ALT0 */
    reg &= ~((7 << 0) | (7 << 3));
    reg |=  ((4 << 0) | (4 << 3));

    writel(reg, gpio_base + GPFSEL1);
}

/* Initialize SPI hardware */
static void spi_init_hw(void)
{
    spi_write(SPI_CS,SPI_CS_CLEAR_RX | SPI_CS_CLEAR_TX);

    /* SPI clock divider */
    spi_write(SPI_CLK,64);
}

/* SPI transfer one byte */
static u8 spi_transfer_byte(u8 data)
{
    while(!(spi_read(SPI_CS) & SPI_CS_TXD))
        cpu_relax();

    spi_write(SPI_FIFO,data);

    while(!(spi_read(SPI_CS) & SPI_CS_RXD))
        cpu_relax();

    return spi_read(SPI_FIFO) & 0xFF;
}

/* Read MCP3208 ADC channel */
static int mcp3208_read_channel(u8 channel)
{
    u8 rx2,rx3;
    int value;

    /* Clear FIFOs */
    spi_write(SPI_CS,SPI_CS_CLEAR_RX | SPI_CS_CLEAR_TX);

    /* Start SPI transfer */
    spi_write(SPI_CS,spi_read(SPI_CS) | SPI_CS_TA);

    spi_transfer_byte(0x06 | ((channel & 0x04) >> 2));

    rx2 = spi_transfer_byte((channel & 0x03) << 6);

    rx3 = spi_transfer_byte(0x00);

    while(!(spi_read(SPI_CS) & SPI_CS_DONE))
        cpu_relax();

    /* Stop transfer */
    spi_write(SPI_CS,spi_read(SPI_CS) & ~SPI_CS_TA);

    value = ((rx2 & 0x0F) << 8) | rx3;

    return value;
}

/* Workqueue function */
static void work_function(struct work_struct *work)
{
    pr_info("%s\n", log_buffer);

    /* Send logs through UART */
    uart_tx_string(log_buffer);
    uart_tx('\n');
    uart_tx('\r');

    /* Wakeup blocked read */
    wake_up_interruptible(&read_queue);
}

/* Timer callback every 2 seconds */
static void timer_callback(struct timer_list *t)
{
    int adc,mv,temp,duty,f;

    /* Read ADC */
    adc = mcp3208_read_channel(MCP3208_CH0);

    /* Convert ADC to millivolt */
    mv = (adc * 3300) / 4095;

    /* LM35 gives 10mV per degree */
    temp = mv / 10;

    /* Celsius to Fahrenheit */
    f = ((temp * 9) / 5) + 32;

    /* LED brightness control */
    if(temp <= 20)
        duty = 0;
    else
        duty = (adc * 1024) / 4095;

    /* Update PWM duty */
    writel(duty,pwm_base + PWMDAT1);
	

if(temp < 20){
    gpio_set_value(LOW,1);
    gpio_set_value(MED,0);
    gpio_set_value(HIGH,0);
}
else if(temp < 50){
    gpio_set_value(MED,1);
    gpio_set_value(LOW,0);
    gpio_set_value(HIGH,0);
}
else {
    gpio_set_value(HIGH,1);    
    gpio_set_value(LOW,0);
    gpio_set_value(MED,0);
}
//snprintf(log_buffer,sizeof(log_buffer),
      //      "TEMP=%dC ADC=%d DUTY=%d F=%d",
        //    temp,adc,duty,f);

ktime_get_real_ts64(&ts);

rtc_time64_to_tm(ts.tv_sec, &tm);

snprintf(log_buffer,sizeof(log_buffer),"[%04ld-%02d-%02d %02d:%02d:%02d] TEMP=%dC ADC=%d DUTY=%d F=%d",tm.tm_year + 1900,tm.tm_mon + 1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,temp,adc,duty,f);
    data_ready = 1;

    /* Schedule workqueue */
    queue_work(my_wq,&my_work);

    /* Restart timer */
    mod_timer(&sample_timer,jiffies + msecs_to_jiffies(2000));
}

/* Device open */
static int my_open(struct inode *inode,struct file *file)
{
    return 0;
}

/* Device close */
static int my_release(struct inode *inode,struct file *file)
{
    return 0;
}

/* Write from user space */
static ssize_t my_write(struct file *file,const char __user *buf,
                        size_t len,loff_t *offset)
{
    char kbuf[16];
    int value,duty;

    if(len > 15)
        len = 15;

    if(copy_from_user(kbuf,buf,len))
        return -EFAULT;

    kbuf[len] = '\0';

    if(kstrtoint(kbuf,10,&value))
        return -EINVAL;

    if(value < 0)
        value = 0;

    if(value > 100)
        value = 100;

    /* Convert percentage to PWM */
    duty = (value * 1024) / 100;

    writel(duty,pwm_base + PWMDAT1);

    return len;
}

/* Read logs from driver */
static ssize_t my_read(struct file *file,char __user *buf,
                       size_t len,loff_t *offset)
{
    /* Sleep until data ready */
    if(wait_event_interruptible(read_queue,data_ready))
        return -ERESTARTSYS;

    data_ready = 0;

    if(copy_to_user(buf,log_buffer,strlen(log_buffer)))
        return -EFAULT;

    return strlen(log_buffer);
}

/* File operations */
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .release = my_release,
    .write   = my_write,
    .read    = my_read,
};
static int LED_INIT(void)
{
	int ret;
	if(!gpio_is_valid(LOW))
	{
		pr_err("GPIO %d is invalid\n",LOW);
		return -ENODEV;
	}
	if(!gpio_is_valid(MED))
	{
		pr_err("GPIO %d is invalid\n",MED);
		return -ENODEV;
	}
	if(!gpio_is_valid(HIGH))
	{
		pr_err("GPIO %d is invalid\n",HIGH);
		return -ENODEV;
	}


	ret=gpio_request(LOW,"LED1");
	if(ret){
		return ret;
	}
	ret=gpio_request(MED,"LED2");
	if(ret){
		return ret;
	}
	ret=gpio_request(HIGH,"LED3");
	if(ret){
		return ret;
	}
	gpio_direction_output(LOW,1);
	gpio_direction_output(MED,1);
	gpio_direction_output(HIGH,1);

	pr_info("Alert system initialized succesfully\n");
	return 0;
}
static void LED_EXIT(void)
{
	gpio_set_value(LOW,0);
	gpio_set_value(MED,0);
	gpio_set_value(HIGH,0);
	gpio_free(LOW);
	gpio_free(MED);
	gpio_free(HIGH);
}
/* Driver probe */
static int my_probe(struct platform_device *pdev)
{
    struct resource *res;
    int irq,ret;
    u32 gpfsel;

    /* Map peripheral memory */
    gpio_base   = ioremap(GPIO_BASE,0x1000);
    pwm_base    = ioremap(PWM_BASE,0x28);
    pwmclk_base = ioremap(PWMCLK_BASE,0x08);
    spi_base    = ioremap(SPI0_BASE,0x100);

    if(!gpio_base || !pwm_base || !pwmclk_base || !spi_base)
        return -ENOMEM;

    /* Get UART resource from DT */
    res = platform_get_resource(pdev,IORESOURCE_MEM,0);

    uart_base = devm_ioremap_resource(&pdev->dev,res);

    if(IS_ERR(uart_base))
        return PTR_ERR(uart_base);

    /* Get IRQ number */
    irq = platform_get_irq(pdev,0);

    /* Register interrupt */
    ret = devm_request_irq(&pdev->dev,irq,
                           uart_irq_handler,
                           0,"my_uart2",NULL);

    if(ret)
        return ret;

    /* UART GPIO config */
    writel(0x1B,gpio_base + GPFSEL0);

    /* UART init */
    writel(0x0,uart_base + CR);

    writel(26,uart_base + IBRD);
    writel(3,uart_base + FBRD);

    writel(0x60,uart_base + LCRH);

    writel(0x7FFF,uart_base + ICR);

    /* Enable RX interrupt */
    writel((1 << 4),uart_base + IMSC);

    /* Enable UART RX TX */
    writel((1 << 0) | (1 << 8) | (1 << 9),
           uart_base + CR);

    uart_tx_string("UART STARTED\n");

    /* PWM GPIO18 ALT5 */
    gpfsel = readl(gpio_base + GPFSEL1);

    gpfsel &= ~(7 << 24);
    gpfsel |=  (2 << 24);

    writel(gpfsel,gpio_base + GPFSEL1);

    /* Stop PWM clock */
    writel(0x5A000000 | 1,pwmclk_base + PWMCLK_CNTL);

    udelay(10);

    /* Set PWM divider */
    writel(0x5A000000 | (19 << 12) | 1,
           pwmclk_base + PWMCLK_DIV);

    /* Start PWM clock */
    writel(0x5A000011,pwmclk_base + PWMCLK_CNTL);

    /* Configure PWM */
    writel(0,pwm_base + PWMCTL);

    udelay(10);

    writel(1024,pwm_base + PWMRNG1);

    writel(0,pwm_base + PWMDAT1);

    /* Enable PWM */
    writel(0x81,pwm_base + PWMCTL);

    /* Initialize SPI */
    spi_gpio_init();
    spi_init_hw();

    /* Character device */
    alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);

    cdev_init(&my_cdev,&fops);

    cdev_add(&my_cdev,dev_num,1);
    my_class = class_create(CLASS_NAME);

    my_device = device_create(my_class,NULL,
                              dev_num,NULL,
                              DEVICE_NAME);

    /* Initialize waitqueue */
    init_waitqueue_head(&read_queue);

    /* Create workqueue */
    my_wq = create_singlethread_workqueue("my_wq");

    INIT_WORK(&my_work,work_function);

    /* Start timer */
    timer_setup(&sample_timer,timer_callback,0);

    mod_timer(&sample_timer,
              jiffies + msecs_to_jiffies(2000));


	LED_INIT();
	gpio_set_value(LOW,1);
    pr_info("FULL DRIVER LOADED\n");

    return 0;
}

/* Driver remove */
static void my_remove(struct platform_device *pdev)
{
    del_timer_sync(&sample_timer);

    flush_workqueue(my_wq);

    destroy_workqueue(my_wq);
	LED_EXIT();
    /* Disable UART */
    writel(0x0,uart_base + CR);

    /* Disable PWM */
    writel(0,pwm_base + PWMCTL);

    /* Stop PWM clock */
    writel(0x5A000000 | 1,pwmclk_base + PWMCLK_CNTL);

    device_destroy(my_class,dev_num);

    class_destroy(my_class);

    cdev_del(&my_cdev);

    unregister_chrdev_region(dev_num,1);

    iounmap(spi_base);
    iounmap(gpio_base);
    iounmap(pwmclk_base);
    iounmap(pwm_base);

    pr_info("DRIVER REMOVED\n");
}

/* Device tree match table */
static const struct of_device_id my_of_match[] = {
    { .compatible = "my_uart2" },
    { }
};

MODULE_DEVICE_TABLE(of,my_of_match);

/* Platform driver */
static struct platform_driver my_driver = {
    .probe  = my_probe,
    .remove = my_remove,
    .driver = {
        .name = "my-uart2",
        .of_match_table = my_of_match,
    },
};

module_platform_driver(my_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh Charan");
MODULE_DESCRIPTION("Smart automated industrial monitoring system");
