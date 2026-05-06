#include<linux/module.h> // for kernel modules  ex: module_init
#include<linux/kernel.h>// printk,pr_info fns
#include<linux/interrupt.h>// interrupt handling functions
#include<linux/io.h>//readl ,writel ,ioremap functions
#include <linux/spi/spi.h>//spi device APIs
#include <linux/gpio/consumer.h>//modern gpio fns
#include <linux/of.h>//device tree support
#include <linux/device.h>//for struct device,driver binding
#include<linux/gpio.h>// gpio fns
#include<linux/delay.h>//for udelay,msleep
#include<linux/workqueue.h>// for workqueue functions
#include<linux/types.h>// data types u8,u16
#include "char_header.h"// characters header

#define DHT_GPIO_4 4+512 //for sensor data output pin
#define GPIO_17 17+512 // gpio interrupt pin

#define TIMER_PHY 0xFE003000 // physical base address of s/s timer register
#define TIMER_CLO  0x04 //  timer counter register
#define TIMER_SIZE 0x1C // size of timer memory map  region 
#define MSEC 1000  // for convert microseconds to milliseconds
#define DRIVER_NAME "ili9225_driver"  //driver name
#define LINE_HEIGHT 20 //pixel height between text lines
#define UART_BASE  0xFE201000 //physical address of uart H/W
#define UART_SIZE  0x100 //size of uart memory map region

#define UART_DR    0x00 //data reg send or recieve data
#define UART_FR    0x18 //flag reg tx full ,Rx empty
#define UART_IBRD  0x24//integer baud rate divisor register
#define UART_FBRD  0x28 // fractional baud rate divisor
#define UART_LCR  0x2C //line control register ,WL,FIFO EN,stop bits
#define UART_CR    0x30 // control reg ,uart EN,TX EN,RX EN
#define UART_IMSC  0x38 //interrupt mask set /clear reg disable uart interrupts
#define UART_ICR   0x44 // interrupt clear reg,clear pending uart interrupts

/* Flags */
#define FR_TXFF (1 << 5) //tx fifo full flag ,1->fifo full ,0->fifo empty
#define FR_RXFE (1 << 4) //rx fifo empty flag,1->empty,0->available data

/* Control bits */
#define CR_UARTEN (1 << 0) //EN uart H/W
#define CR_TXE    (1 << 8) //EN Tx
#define CR_RXE    (1 << 9) // EN RX

/* Line control */
#define LCR_8BIT (3 << 5)//select 8 bit length
#define LCR_FEN  (1 << 4) //EN tx fifo and rx fifo buffers


#define LCD_WIDTH   220 // horizontal direction
#define LCD_HEIGHT  176 // vertical direction

struct ili9225_display {
    struct spi_device *spi;//used to communicate with display via SPI bus
    struct gpio_desc *rs;// register select  rs=0 ->command rs=1 ->data //gpio controller,flags,consumer name
    struct gpio_desc *reset;//reset the dislpay
};
static struct ili9225_display *g_lcd,*lcd;

static int irq;//store interrupt number assigned to GPIO pin
static struct workqueue_struct *my_wq;// pointer to work queue
static struct work_struct my_work;// used for to do assigned task later
static void __iomem *timer_base; //pointer to memory mapped h/w timer registers
static void __iomem *uart_base;

static void delay_us(unsigned int us)
{
/* it will generate delay in microseconds*/
u32 start,now;
start=readl(timer_base+TIMER_CLO);//read current time counter value
while(1)
{
now=readl(timer_base+TIMER_CLO); //reading continously timer counter value 
if((now-start)>=us) // if required delay comes break it
	break;
}
/* ex: start =10000
us=5000;
now=150000-10000 =5000 -> break the loop*/
}

/* 
	Storing data into buffer and sending to a pointer that represents hardware
*/
static int ili9225_display_write16(struct ili9225_display *lcd, u16 value)
{
    u8 buf[2];
    buf[0] = value >> 8;
    buf[1] = value & 0xFF;
    return spi_write(lcd->spi, buf, 2);
}


static int ili9225_display_write_reg(struct ili9225_display *lcd, u16 reg, u16 data)
{
/*---To send command ---*/
    gpiod_set_value(lcd->rs, 0);
    ili9225_display_write16(lcd, reg);
/*---To send data---*/
    gpiod_set_value(lcd->rs, 1);
    return ili9225_display_write16(lcd, data);
}






static void ili9225_display_reset(struct ili9225_display *lcd)
{
    gpiod_set_value(lcd->reset, 1);   //pull reset pin high
    msleep(5);                         //wait fo 5ms
    gpiod_set_value(lcd->reset, 0);    //pull low(Trigger reset)
    msleep(20);                        //wait for 20 ms
    gpiod_set_value(lcd->reset, 1);    //again pull high(Release reset pin)
    msleep(50);                        //Final stabilization delay
}





static void ili9225_display_init(struct ili9225_display *lcd)
{
    ili9225_display_reset(lcd);                           //Resetting the LCD to clear previous logics(If any)

    ili9225_display_write_reg(lcd, 0x0001, 0x011C);       //Scan the screen from top to bottom
    ili9225_display_write_reg(lcd, 0x0002, 0x0100);       //Allows flipping of voltage between pixels

    // LANDSCAPE ENTRY MODE
    ili9225_display_write_reg(lcd, 0x0003, 0x1038);       //Tells address counter to increment horizantally(Landscape)

    ili9225_display_write_reg(lcd, 0x0008, 0x0808);       //Controls idle time between frames
    ili9225_display_write_reg(lcd, 0x000C, 0x0000);       //Selects internal clock
    ili9225_display_write_reg(lcd, 0x000F, 0x0B01);       //Controls clock frequency

    ili9225_display_write_reg(lcd, 0x0010, 0x0A00);       //Sets internal voltage
    ili9225_display_write_reg(lcd, 0x0011, 0x1038);       //Enables internal circuits and starts charge pump
    msleep(50);                                           //wait for 50ms to get voltage stability

    ili9225_display_write_reg(lcd, 0x0012, 0x1121);       //Sets reference voltage for pixels brightness
    ili9225_display_write_reg(lcd, 0x0013, 0x0063);       //Controls voltage switching
    ili9225_display_write_reg(lcd, 0x0014, 0x5A00);       //Fine tuning
    msleep(50);                                           //Delay for stabilizing voltage

    ili9225_display_write_reg(lcd, 0x0007, 0x1017);       //Enables Internal logic, Display output
    msleep(20);                                           //Delay to ensure stable start
}





static void ili9225_display_fill(struct ili9225_display *lcd, u16 color)
{
    int x, y;

    ili9225_display_write_reg(lcd, 0x0036, LCD_HEIGHT - 1);   //Sets vertical bottom limit
    ili9225_display_write_reg(lcd, 0x0037, 0);                //sets vertical top limit
    ili9225_display_write_reg(lcd, 0x0038, LCD_WIDTH - 1);    //sets horizontal end 
    ili9225_display_write_reg(lcd, 0x0039, 0);                //sets horizantal start

    ili9225_display_write_reg(lcd, 0x0020, 0);                //Sets coloumn pointer to 0
    ili9225_display_write_reg(lcd, 0x0021, 0);               //Sets row pointer to 0

    gpiod_set_value(lcd->rs, 0);                             //Setting low (To send ommand)
    ili9225_display_write16(lcd, 0x0022);                    //This is GRAM data register
    gpiod_set_value(lcd->rs, 1);                            //Setting high to send data

    for (y = 0; y < LCD_HEIGHT; y++)
        for (x = 0; x < LCD_WIDTH; x++)
            ili9225_display_write16(lcd, color);             //Filling screen with (white) background
}


static void drawPixel(int x, int y, uint16_t color)
{
    struct ili9225_display *lcd = g_lcd;                 //pointer to LCD device

    // New rotation for the circled corner
    int x_rotated = (LCD_WIDTH - 1) - x;                 //To write data in landscape mode
    int y_rotated = y;                                   

    gpiod_set_value(lcd->rs, 0);                   
    ili9225_display_write16(lcd, 0x0020);                //Setting row position
    gpiod_set_value(lcd->rs, 1);
    ili9225_display_write16(lcd, y_rotated); // Use y_rotated for Gram Address Set

    gpiod_set_value(lcd->rs, 0);
    ili9225_display_write16(lcd, 0x0021);                //Setting coloumn position 
    gpiod_set_value(lcd->rs, 1);
    ili9225_display_write16(lcd, x_rotated); // Use x_rotated for Gram Address Set
/////////////Setting colour//////////////////////////////
    gpiod_set_value(lcd->rs, 0);
    ili9225_display_write16(lcd, 0x0022);
    gpiod_set_value(lcd->rs, 1);
    ili9225_display_write16(lcd, color);
}
static void drawChar(int x, int y, unsigned char c, u16 color)
{
int scale=2;
    if (c < 32 || c > 126)               //Allow printable charecters
        return;

    const unsigned char *bitmap = font8x8[c - 32];  //storing the charecter

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            if (bitmap[row] & (1 << (7 - col)))      //1->draw pixel,0->skip pixel
            {
                for (int dy = 0; dy < scale; dy++)
                {
                    for (int dx = 0; dx < scale; dx++)
                    {
                        drawPixel(x + col * scale + dx, y + row * scale + dy, color);
                    }
                }
            }
        }
    }
}
static void drawString(int x, int y, const unsigned char *str,u16 color)
{
    int scale = 2;

    for (int i = 0; str[i]; i++)
    {
        drawChar(x, y, str[i], color);
        x += (8*scale)+2;   // adjust spacing
    }
}
static void sensor_work(struct work_struct *work)
{
 int i, j;
    u8 buf[5] = {0}; // buffers to store sensor data of 5 bytes
    char temp_str[25],hum_str[25],str[60]; //strings to store temperature and humidity

   /* Start condition */
    gpio_direction_output(DHT_GPIO_4, 0); // pull pin to  low to start communication
   delay_us(18*MSEC);    //wait for 18 msec
 
      gpio_set_value(DHT_GPIO_4, 1);//  pull pin to high 
      delay_us(35); //wait for 35us (20-40us)
    gpio_direction_input(DHT_GPIO_4); //changing GPIO pin to input to recieve data from sensor 

    if (gpio_get_value(DHT_GPIO_4))  // checking pin  low or not
        pr_info("GPIO_4 error in sensor work\n");
/*sensor response waiting time*/
    while (!gpio_get_value(DHT_GPIO_4));// pin will be in low for 80us 
    while (gpio_get_value(DHT_GPIO_4)); // pin will be in high for 80us 

    /* Read 40 bits */
    for (i = 0; i < 5; i++) {
        for (j = 0; j< 8; j++) {

            while (!gpio_get_value(DHT_GPIO_4)); //wait for start bit high

            delay_us(30); //wait for 30us to detect bit value (26-28us)
            if (gpio_get_value(DHT_GPIO_4)) // if still bit is high store value into buffer (pin should be high for 70us)
                buf[i] |= (1 << (7 - j));

            while (gpio_get_value(DHT_GPIO_4)); //wait for pin to low
        }
    }

    /* Checksum */
    if (((buf[0] + buf[1] + buf[2] + buf[3]) & 0xFF) != buf[4]) //checking the data  valid or not
        pr_info("sensor data error\n");

pr_info("Temperature=%d.%d°C\n",buf[2],buf[3]);//tempearture -> buf[2]=integer part ,buf[3]=decimal part
pr_info("Humidity=%d.%d%%\n",buf[0],buf[1]);// humidity -> buf[0]=integer part buf[1]=fraction part 
  ili9225_display_fill(g_lcd, 0xFFFF);// clear screen with white screen
snprintf(temp_str,sizeof(temp_str),"%d.%d%cC",buf[2],buf[3],126); //converting temperature into string
snprintf(hum_str,sizeof(hum_str),"%d.%d%%",buf[0],buf[1]);// converting humidity into string

drawString(5,10,"Temperature:",0x0000);
drawString(5,40,temp_str,0x0000);
drawString(5,70,"Humidity:",0x0000);
drawString(5,100,hum_str,0x0000);
snprintf(str,sizeof(str),"Temperature:%d.%d°C\nHumidity:%d.%d%%\n",buf[2],buf[3],buf[0],buf[1]);  

for(i=0;str[i];i++)
{
while(readl(uart_base+UART_FR)&FR_TXFF);
writel(str[i],uart_base+UART_DR);
}

enable_irq(irq);// enable irq
}
static irqreturn_t GPIO_isr(int irq, void *dev_id)
{
	disable_irq_nosync(irq); //avoiding multiple triggers
	pr_info("GPIO interrrupt triggered\n");
	queue_work(my_wq, &my_work);// schedule the work
	return IRQ_HANDLED;
}
static void uart_hw_init(void)
{
writel(0,uart_base+UART_CR); //disable uart
writel(0x7FF,uart_base+UART_ICR); //clear irqs

writel(26,uart_base+UART_IBRD); //integer baud rate
writel(3,uart_base+UART_FBRD); //fraction baud rate

writel(LCR_8BIT|LCR_FEN,uart_base+UART_LCR);//wl=8,fifo EN

writel(CR_UARTEN|CR_TXE|CR_RXE,uart_base+UART_CR);//uart EN,TX EN,RX EN
writel(0,uart_base+UART_IMSC);//no interupts
    pr_info(" UART initialized\n");
}


/* ---------------------------------------------------- */
static int ili9225_display_probe(struct spi_device *spi)
{
    lcd = devm_kzalloc(&spi->dev, sizeof(*lcd), GFP_KERNEL); //allocate memory for struct ili9225 and automatically freed when driver is removed
    if (!lcd)
{
 pr_info("memory not allocated\n");
       return 0;
}
    lcd->spi = spi; //store the SPI device pointer to our structure (in driver)
    spi_set_drvdata(spi, lcd);//store lcd structure inside SPI device (in kernel)

    lcd->rs = devm_gpiod_get(&spi->dev, "rs", GPIOD_OUT_LOW);// get GPIO pin from DT and set to low
    lcd->reset = devm_gpiod_get(&spi->dev, "reset", GPIOD_OUT_HIGH);//get GPIO pin from DT and set to high

    spi->mode = SPI_MODE_0; // cpol=0,cpha=0
    spi->bits_per_word = 8;// 1 byte at a time per transfer
    spi_setup(spi);//applying spi configuration to hardware controller

    ili9225_display_init(lcd);// initailizing display
    ili9225_display_fill(lcd, 0xFFFF);// fill screen with white color

    g_lcd = lcd; //save pointer to globally
	if(!gpio_is_valid(GPIO_17))//  checking 17 present or not
		pr_err("Invalid GPIO 17\n");
	int rt=gpio_request(GPIO_17,"button"); // 17 as button
	if(rt)
		pr_info("gpio req failed\n");
	gpio_direction_input(GPIO_17); //set  GPIO 17 as input
	irq=gpio_to_irq(GPIO_17); //convert gpio number to irq  number
	int ret=request_irq(irq,GPIO_isr,IRQF_TRIGGER_RISING,"GPIO_ISR",NULL); //registering ISR  when button is pressed
	if(ret){
		pr_err("request irq failed\n");
		return ret;
	}

        if(!gpio_is_valid(DHT_GPIO_4)) //checking GPIO 4 present or not
          pr_err("GPIO 4 invalid\n");

	/* GPIO request */
	ret = gpio_request(DHT_GPIO_4, "dht11_gpio");//setting GPIO 4  as sensor data pin
	if (ret) {
		pr_err("GPIO request failed\n");
		return ret;
	}
/* Create Workqueue */
    my_wq = create_singlethread_workqueue("dht_wq"); // creating worker thread
    if (!my_wq)
        return -ENOMEM;

    INIT_WORK(&my_work, sensor_work); //initializing sensor work  function

	gpio_direction_output(DHT_GPIO_4, 1); //set as output for sensor data pin

 timer_base=ioremap(TIMER_PHY,TIMER_SIZE);// map physical timer register address into kernel virtual memory address
    if (!timer_base) {
        pr_err("ioremap failed\n");
        return 0;
    }
	pr_info("module loaded\n");

uart_base = ioremap(UART_BASE, UART_SIZE);//
    if (!uart_base) {
        pr_err("rpi_uart: ioremap failed\n");
        return -ENOMEM;
    }


    uart_hw_init(); // intailizing uart H/W
	return 0;
}


static void ili9225_display_remove(struct spi_device *spi)
{
	disable_irq(irq); // disabling interrupt
	gpio_set_value(GPIO_17,0);// set gpio17 to 0
	gpio_free(GPIO_17);//free the pin
	pr_info("GPIO_17 freed\n");

	gpio_free(DHT_GPIO_4); //free the pin GPIO 4
	pr_info("GPIO_4 freed\n");
	free_irq(irq,NULL);//release the ISR
	
	flush_workqueue(my_wq);//for finishing the pending work
	destroy_workqueue(my_wq); // delete workqueue

if (timer_base){
        iounmap(timer_base);}//unmap memory mapped timer registers
iounmap(uart_base);
pr_info("removed\n");

}
static const struct of_device_id ili9225_dt_ids[] = {
    { .compatible = "ili9225_display" },// device tree compatible string to match driver
    { }
};
static struct spi_driver ili9225_display_driver = {
    .driver = {
        .name = DRIVER_NAME, // driver name
        .of_match_table = ili9225_dt_ids, //device tree match table
    },
    .probe  = ili9225_display_probe, //it willlbe called when device is connected
    .remove = ili9225_display_remove, //it will be called when device is removed
};
MODULE_DEVICE_TABLE(of, ili9225_dt_ids); // auto module loading using device tree match
module_spi_driver(ili9225_display_driver); //register SPI driver

MODULE_LICENSE("GPL"); //avoids kernel warinings
MODULE_AUTHOR("team2");//author information of module
MODULE_DESCRIPTION("GPIO + DHT11 + workqueue+timer interrupt + ioremap ");//description about driver
