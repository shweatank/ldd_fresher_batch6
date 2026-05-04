#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include<linux/workqueue.h>
#include<linux/delay.h>
#include<linux/interrupt.h>

#define UART2_BASE 0xFE201400
#define GPIO_BASE 0xFE200000



#define DR 0x00
#define FR 0x18
#define IBRD 0x24
#define FBRD 0x28
#define LCRH 0x2c
#define CR 0x30
#define ICR 0x44
//#define IMSC 0x38
//#define MIS 0x40
//#define ICR 0x44

#define IMSC 0x38
#define MIS 0x40
#define ICR 0x44
#define UART2_IRQ 121


//#define GPFSEL1 0x04
#define GPFSEL0 0x00
#define GPFSEL2 0x08

#define GPFSET0 0x1c
#define GPFCLR0 0x28

#define GPIO_LED (512+21)
#define GPSET0 0x1c
//#define GPCLR0 0x20

#define FR_TX (1<<5)
#define FR_RX (1<<4)

//static void uart_rx_string(char *str);
//static char string[10];
//static struct work_struct my_work;

static void __iomem* uart_base;
static void __iomem *gpio_base;

static char uart_rx(void);

static void uart_tx_string(char *str);
static void uart_tx(char ch);
/*static void my_work_handler(struct work_struct *work)
{
	pr_info("In My workq handler\n");
	gpio_set_value(GPIO_LED,1);
//	uart_tx_string("\n\rWaiting for data:");
//	uart_rx_string(string);
	uart_tx('1');
//	udelay(100);
	char ch=uart_rx();
	if(ch=='1')
		pr_info("Uart loop back is working\n");
	
//	pr_info("Received string %s\n",string);
	for(int i=0;i<10;i++){
	char ch=uart_rx();
	if(ch=='1')
		writel((1<<21),gpio_base+GPSET0);
	else if(ch=='0')
		writel((1<<21),gpio_base+GPFCLR0);
	else	
		writel((1<<21),gpio_base+GPFCLR0);
	pr_info("Received %c\n",ch);
	}

}*/
static void uart_tx(char ch)
{
	while( (readl(uart_base+FR))& (FR_TX))
		cpu_relax();
	 
	writel(ch,uart_base+DR);
}
/*static char uart_rx(void)
{
	while((readl(uart_base+FR) & FR_RX ))
		cpu_relax();
	return readl(uart_base+DR) & 0xff;

}*/
/*static void uart_rx_string(char *str)
{
	for(int i=0;i<8;i++)
		str[i]=uart_rx();
}*/

static void uart_tx_string(char *str)
{
	while(*str)
		uart_tx(*str);
}
/*static void led_blink(void)
{
	uart_tx('a');
	char ch=uart_rx();
//	gpio_set_value(GPIO_LED,1);
	if(ch=='a')
		gpio_set_value(GPIO_LED,1);
	else
		gpio_set_value(GPIO_LED,0);
	pr_info("Come in led_blink is led is blinking ...?\n");

}	*/
static char str[10];
static int index;
static irqreturn_t my_handler(int irq,void *dev_id)
{
	gpio_set_value(GPIO_LED,1);
	unsigned int status=readl(uart_base+MIS);
	if(status & (1<<4))
	{

	char ch=readl(uart_base+DR)&0xff;

			str[index++] = ch;
			if (index >= 9)
    			index = 0;
			str[index] = '\0';
	
	writel(1<<4,uart_base+ICR);
	
		pr_info("Uart rx handler data received %s\n",str);
	}
return IRQ_HANDLED;
}

static int __init my_init(void)
{
	pr_info("In init\n");
	uart_base=ioremap(UART2_BASE,0x90);
	if(!uart_base)
	{
		pr_err("Failed to allocate memory\n");
		return -ENOMEM;
	}
	gpio_base=ioremap(GPIO_BASE,0x90);
	if(!gpio_base)
	{
		pr_err("Failed to create memmory gpio\n");
		return -ENOMEM;
	}
	/* uart */
	//  GPIO0-> txd2 GPIO1->rxd2

	int reg=readl(gpio_base+GPFSEL0);
	reg &= ~((7<<3) | (7<<0));
	reg |= (3<<3)|(3<<0);
	
	writel(reg,gpio_base+GPFSEL0);
	
	writel(0x0,uart_base+CR);
	/* baud rate */
		//9600
	writel(26,uart_base+IBRD);
	writel(3,uart_base+FBRD);
	/* 8 data bits and fifo enabled */
	writel(0x70,uart_base+LCRH);

	/*	Interrupt Section intr number 153 */
	writel(0x7ff,uart_base+ICR);
	writel(1<<4,uart_base+IMSC);
	int ret;
	ret=request_irq(UART2_IRQ,my_handler,0,"uart2_irq",NULL);
	if(ret)
	{
		pr_err("Failed to  request irq\n");
		return ret;
	}

	
	/*  uart enable */
	writel((1<<0)|(1<<8)|(1<<9),uart_base+CR);

	/*	LED	*/
	
	int f=readl(gpio_base+GPFSEL2);
	f &= ~(7<<3);
	f |= (1<<3); // gpio output direction
	writel(f,gpio_base+GPFSEL2);	
//	uart_tx_string("\n\r Uart initialised successfully\n");
	
	

	//led_blink();
	//INIT_WORK(&my_work,my_work_handler);
	//schedule_work(&my_work);
	
	return 0;
}

static void __exit my_exit(void)
{
	pr_info("UART exit\n");
	//flush_work(&my_work);
	free_irq(UART2_IRQ,NULL);
	gpio_set_value(GPIO_LED,0);
	iounmap(uart_base);
	iounmap(gpio_base);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("UART2");

