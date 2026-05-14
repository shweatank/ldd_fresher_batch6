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
#include <linux/string.h>

/* ========================================================= */
/* DEVICE INFO                                               */
/* ========================================================= */

#define DEVICE_NAME "pwm_uart_spi"
#define CLASS_NAME  "my_class"

/* ========================================================= */
/* PERIPHERAL BASE                                           */
/* ========================================================= */

#define PERI_BASE      0xFE000000UL

#define GPIO_BASE      (PERI_BASE + 0x200000)
#define SPI0_BASE      (PERI_BASE + 0x204000)

#define PWM_BASE       0xFE20C000
#define PWMCLK_BASE    0xFE1010A0

#define UART2_BASE     0xFE201400

/* ========================================================= */
/* PWM REGISTERS                                             */
/* ========================================================= */

#define PWMCTL         0x00
#define PWMSTA         0x04
#define PWMDMAC        0x08
#define PWMRNG1        0x10
#define PWMDAT1        0x14
#define PWMFIF1        0x18
#define PWMRNG2        0x20
#define PWMDAT2        0x24

#define PWMCLK_CNTL    0x00
#define PWMCLK_DIV     0x04

/* ========================================================= */
/* GPIO REGISTERS                                            */
/* ========================================================= */

#define GPFSEL0        0x00
#define GPFSEL1        0x04

/* ========================================================= */
/* SPI0 REGISTERS                                            */
/* ========================================================= */

#define SPI_CS         0x00
#define SPI_FIFO       0x04
#define SPI_CLK        0x08

#define SPI_CS_TXD        (1 << 18)
#define SPI_CS_RXD        (1 << 17)
#define SPI_CS_DONE       (1 << 16)

#define SPI_CS_TA         (1 << 7)

#define SPI_CS_CLEAR_RX   (1 << 5)
#define SPI_CS_CLEAR_TX   (1 << 4)

/* ========================================================= */
/* UART2 REGISTERS                                           */
/* ========================================================= */

#define DR      0x00
#define FR      0x18
#define IBRD    0x24
#define FBRD    0x28
#define LCRH    0x2C
#define CR      0x30
#define IMSC    0x38
#define MIS     0x40
#define ICR     0x44

#define FR_RX   (1 << 4)
#define FR_TX   (1 << 5)

/* ========================================================= */

#define MCP3208_CH0 0

/* ========================================================= */
/* GLOBAL VARIABLES                                          */
/* ========================================================= */

static void __iomem *uart_base;
static void __iomem *gpio_base;
static void __iomem *pwm_base;
static void __iomem *pwmclk_base;
static void __iomem *spi_base;

static char rx_buffer[64];
static int rx_index;

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;

static struct timer_list sample_timer;

static struct workqueue_struct *my_wq;

static struct work_struct my_work;

static wait_queue_head_t read_queue;

static int data_ready;

static char log_buffer[256];

/* ========================================================= */
/* UART TX                                                   */
/* ========================================================= */

static void uart_tx(char ch)
{
    while ((readl(uart_base + FR)) & FR_TX)
        cpu_relax();

    writel(ch, uart_base + DR);
}

static void uart_tx_string(char *str)
{
    while (*str)
        uart_tx(*str++);
}

/* ========================================================= */
/* UART ISR                                                  */
/* ========================================================= */

static irqreturn_t uart_irq_handler(int irq, void *dev_id)
{
    unsigned int status;

    status = readl(uart_base + MIS);

    pr_info("UART ISR\n");

    if (status & (1 << 4)) {

        char ch;

        ch = readl(uart_base + DR);

        if (rx_index < 63) {

            rx_buffer[rx_index++] = ch;

            rx_buffer[rx_index] = '\0';
        }

        pr_info("RECEIVED : %c\n", ch);

        writel((1 << 4), uart_base + ICR);
    }

    return IRQ_HANDLED;
}

/* ========================================================= */
/* SPI READ                                                  */
/* ========================================================= */

static inline u32 spi_read(u32 reg)
{
    return readl(spi_base + reg);
}

/* ========================================================= */
/* SPI WRITE                                                 */
/* ========================================================= */

static inline void spi_write(u32 reg, u32 value)
{
    writel(value, spi_base + reg);
}

/* ========================================================= */
/* SPI GPIO CONFIG                                           */
/* ========================================================= */

static void spi_gpio_init(void)
{
    u32 reg;

    reg = readl(gpio_base + GPFSEL0);

    reg &= ~((7 << 21) |
             (7 << 24) |
             (7 << 27));

    reg |= ((4 << 21) |
            (4 << 24) |
            (4 << 27));

    writel(reg, gpio_base + GPFSEL0);

    reg = readl(gpio_base + GPFSEL1);

    reg &= ~((7 << 0) |
             (7 << 3));

    reg |= ((4 << 0) |
            (4 << 3));

    writel(reg, gpio_base + GPFSEL1);

    pr_info("SPI GPIO CONFIGURED\n");
}

/* ========================================================= */
/* SPI INIT                                                  */
/* ========================================================= */

static void spi_init_hw(void)
{
    spi_write(SPI_CS,
              SPI_CS_CLEAR_RX |
              SPI_CS_CLEAR_TX);

    spi_write(SPI_CLK, 64);

    pr_info("SPI INITIALIZED\n");
}

/* ========================================================= */
/* SPI TRANSFER                                              */
/* ========================================================= */

static u8 spi_transfer_byte(u8 data)
{
    while (!(spi_read(SPI_CS) & SPI_CS_TXD))
        cpu_relax();

    spi_write(SPI_FIFO, data);

    while (!(spi_read(SPI_CS) & SPI_CS_RXD))
        cpu_relax();

    return spi_read(SPI_FIFO) & 0xFF;
}

/* ========================================================= */
/* MCP3208 READ                                              */
/* ========================================================= */

static int mcp3208_read_channel(u8 channel)
{
    u8 rx1;
    u8 rx2;
    u8 rx3;

    int value;

    spi_write(SPI_CS,
              SPI_CS_CLEAR_RX |
              SPI_CS_CLEAR_TX);

    spi_write(SPI_CS,
              spi_read(SPI_CS) | SPI_CS_TA);

    rx1 = spi_transfer_byte(0x06 |
           ((channel & 0x04) >> 2));

    rx2 = spi_transfer_byte(
           (channel & 0x03) << 6);

    rx3 = spi_transfer_byte(0x00);

    while (!(spi_read(SPI_CS) & SPI_CS_DONE))
        cpu_relax();

    spi_write(SPI_CS,
              spi_read(SPI_CS) &
              ~SPI_CS_TA);

    value = ((rx2 & 0x0F) << 8) | rx3;

    return value;
}

/* ========================================================= */
/* WORKQUEUE FUNCTION                                        */
/* ========================================================= */

static void work_function(struct work_struct *work)
{
    pr_info("WORKQUEUE : %s\n", log_buffer);

    uart_tx_string(log_buffer);

    uart_tx('\n');
    uart_tx('\r');

    wake_up_interruptible(&read_queue);
}

/* ========================================================= */
/* TIMER CALLBACK                                            */
/* ========================================================= */

static void timer_callback(struct timer_list *t)
{
    int adc;
	int f;
    int mv;

    int temp;

    int duty;

    adc = mcp3208_read_channel(MCP3208_CH0);

   mv = (adc * 3300) / 4095;
	
    temp = mv / 10;
	f=((temp*9)/5)+32;
    if (temp <= 20)
        duty = 0;
	else 
	duty=(adc*1024)/4095;


    writel(duty, pwm_base + PWMDAT1);

    snprintf(log_buffer,
             sizeof(log_buffer),
             "TEMP=%dC ADC=%d DUTY=%d Fahrenheit=%d",
             temp,
             adc,
             duty,f);

    data_ready = 1;

    queue_work(my_wq, &my_work);

    mod_timer(&sample_timer,
              jiffies + msecs_to_jiffies(2000));
}

/* ========================================================= */
/* FILE OPERATIONS                                           */
/* ========================================================= */

static int my_open(struct inode *inode,
                   struct file *file)
{
    pr_info("DEVICE OPENED\n");

    return 0;
}

static int my_release(struct inode *inode,
                      struct file *file)
{
    pr_info("DEVICE CLOSED\n");

    return 0;
}

/* ========================================================= */
/* WRITE                                                     */
/* ========================================================= */

static ssize_t my_write(struct file *file,
                        const char __user *buf,
                        size_t len,
                        loff_t *offset)
{
    char kbuf[16];

    int value;

    int duty;

    if (len > 15)
        len = 15;

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;

    kbuf[len] = '\0';

    if (kstrtoint(kbuf, 10, &value))
        return -EINVAL;

    if (value < 0)
        value = 0;

    if (value > 100)
        value = 100;

    duty = (value * 1024) / 100;

    writel(duty, pwm_base + PWMDAT1);

    pr_info("PWM DUTY=%d\n", duty);

    return len;
}

/* ========================================================= */
/* READ                                                      */
/* ========================================================= */

static ssize_t my_read(struct file *file,
                       char __user *buf,
                       size_t len,
                       loff_t *offset)
{
    int ret;

    wait_event_interruptible(read_queue,
                             data_ready);

    data_ready = 0;

    ret = copy_to_user(buf,
                       log_buffer,
                       strlen(log_buffer));

    if (ret)
        return -EFAULT;

    return strlen(log_buffer);
}

/* ========================================================= */
/* FILE OPERATIONS STRUCTURE                                 */
/* ========================================================= */

static struct file_operations fops = {

    .owner   = THIS_MODULE,
    .open    = my_open,
    .release = my_release,
    .write   = my_write,
    .read    = my_read,
};

/* ========================================================= */
/* PROBE                                                     */
/* ========================================================= */

static int my_probe(struct platform_device *pdev)
{
    struct resource *res;

    int irq;

    int ret;

    u32 gpfsel;

    pr_info("UART DRIVER PROBE\n");

    /* ===================================================== */
    /* IOREMAP                                               */
    /* ===================================================== */

    gpio_base = ioremap(GPIO_BASE, 0x1000);

    if (!gpio_base)
        return -ENOMEM;

    pwm_base = ioremap(PWM_BASE, 0x28);

    if (!pwm_base)
        return -ENOMEM;

    pwmclk_base = ioremap(PWMCLK_BASE, 0x08);

    if (!pwmclk_base)
        return -ENOMEM;

    spi_base = ioremap(SPI0_BASE, 0x100);

    if (!spi_base)
        return -ENOMEM;

    /* ===================================================== */
    /* UART RESOURCE                                         */
    /* ===================================================== */

    res = platform_get_resource(pdev,
                                IORESOURCE_MEM,
                                0);

    uart_base = devm_ioremap_resource(&pdev->dev,
                                      res);

    if (IS_ERR(uart_base))
        return PTR_ERR(uart_base);

    /* ===================================================== */
    /* IRQ                                                   */
    /* ===================================================== */

    irq = platform_get_irq(pdev, 0);

    if (irq < 0)
        return irq;

    ret = devm_request_irq(&pdev->dev,
                           irq,
                           uart_irq_handler,
                           0,
                           "my_uart2",
                           NULL);

    if (ret) {

        dev_err(&pdev->dev,
                "IRQ FAILED\n");

        return ret;
    }

    /* ===================================================== */
    /* GPIO UART CONFIG                                      */
    /* ===================================================== */

    writel(0x1B, gpio_base + GPFSEL0);

    /* ===================================================== */
    /* UART INIT                                             */
    /* ===================================================== */

    writel(0x0, uart_base + CR);

    writel(26, uart_base + IBRD);

    writel(3, uart_base + FBRD);

    writel(0x60, uart_base + LCRH);

    writel(0x7FFF, uart_base + ICR);

    writel((1 << 4), uart_base + IMSC);

    writel((1 << 0) |
           (1 << 8) |
           (1 << 9),
           uart_base + CR);

    uart_tx_string("UART STARTED\n");

    /* ===================================================== */
    /* PWM GPIO CONFIG                                       */
    /* ===================================================== */

    gpfsel = readl(gpio_base + GPFSEL1);

    gpfsel &= ~(7 << 24);

    gpfsel |= (2 << 24);

    writel(gpfsel, gpio_base + GPFSEL1);

    /* ===================================================== */
    /* PWM CLOCK                                             */
    /* ===================================================== */

    writel(0x5A000000 | 1,
           pwmclk_base + PWMCLK_CNTL);

    udelay(10);

    writel(0x5A000000 |
           (19 << 12) |
           1,
           pwmclk_base + PWMCLK_DIV);

    writel(0x5A000011,
           pwmclk_base + PWMCLK_CNTL);

    /* ===================================================== */
    /* PWM CONFIG                                            */
    /* ===================================================== */

    writel(0, pwm_base + PWMCTL);

    udelay(10);

    writel(1024, pwm_base + PWMRNG1);

    writel(0, pwm_base + PWMDAT1);

    writel(0x81, pwm_base + PWMCTL);

    /* ===================================================== */
    /* SPI INIT                                              */
    /* ===================================================== */

    spi_gpio_init();

    spi_init_hw();

    /* ===================================================== */
    /* CHARACTER DEVICE                                      */
    /* ===================================================== */

    ret = alloc_chrdev_region(&dev_num,
                              0,
                              1,
                              DEVICE_NAME);

    if (ret < 0)
        return ret;

    cdev_init(&my_cdev, &fops);

    ret = cdev_add(&my_cdev,
                   dev_num,
                   1);

    if (ret < 0)
        return ret;

    my_class = class_create(CLASS_NAME);

    if (IS_ERR(my_class))
        return PTR_ERR(my_class);

    my_device = device_create(my_class,
                              NULL,
                              dev_num,
                              NULL,
                              DEVICE_NAME);

    if (IS_ERR(my_device))
        return PTR_ERR(my_device);

    /* ===================================================== */
    /* WAIT QUEUE                                            */
    /* ===================================================== */

    init_waitqueue_head(&read_queue);

    /* ===================================================== */
    /* WORKQUEUE                                             */
    /* ===================================================== */

    my_wq = create_singlethread_workqueue("my_wq");

    INIT_WORK(&my_work,
              work_function);

    /* ===================================================== */
    /* TIMER                                                 */
    /* ===================================================== */

    timer_setup(&sample_timer,
                timer_callback,
                0);

    mod_timer(&sample_timer,
              jiffies + msecs_to_jiffies(2000));

    pr_info("FULL DRIVER LOADED\n");

    return 0;
}

/* ========================================================= */
/* REMOVE                                                    */
/* ========================================================= */

static void my_remove(struct platform_device *pdev)
{
    del_timer_sync(&sample_timer);

    flush_workqueue(my_wq);

    destroy_workqueue(my_wq);

    writel(0x0, uart_base + CR);

    writel(0, pwm_base + PWMCTL);

    writel(0x5A000000 | 1,
           pwmclk_base + PWMCLK_CNTL);

    device_destroy(my_class,
                   dev_num);

    class_destroy(my_class);

    cdev_del(&my_cdev);

    unregister_chrdev_region(dev_num,
                             1);

    iounmap(spi_base);

    iounmap(gpio_base);

    iounmap(pwmclk_base);

    iounmap(pwm_base);

    pr_info("DRIVER REMOVED\n");
}

/* ========================================================= */
/* DEVICE TREE MATCH                                         */
/* ========================================================= */

static const struct of_device_id my_of_match[] = {

    { .compatible = "my_uart2" },

    { }
};

MODULE_DEVICE_TABLE(of, my_of_match);

/* ========================================================= */
/* PLATFORM DRIVER                                           */
/* ========================================================= */

static struct platform_driver my_driver = {

    .probe  = my_probe,

    .remove = my_remove,

    .driver = {

        .name = "my-uart2",

        .of_match_table = my_of_match,
    },
};

module_platform_driver(my_driver);

/* ========================================================= */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("UART + PWM + SPI + TIMER + WORKQUEUE + WAITQUEUE DRIVER");
