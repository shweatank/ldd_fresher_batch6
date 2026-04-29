#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>


#define UART2_BASE 0xFE201400
#define GPIO_BASE 0xfe200000



#define DR 0x00
#define FR 0x04
#define IBRD 0x24
#define FBRD 0x28
#define LCRH 0x2c
#define CR 0x30
#define ICR 0x44

		//#define GPFSEL1 0x04
#define GPFSEL0 0x00
#define GPFSEL2 0x08

#define GPFSET0 0x1c
#define GPFCLR0 0x28

#define GPIO_LED (512+27)

#define FR_TX (1<<5)
#define FR_RX (1<<4)

static void __iomem* uart_base;
static void __iomem *gpio_base;

static void uart_tx(char ch)
{
	while( (readl(uart_base+FR))& (FR_TX))
		cpu_relax();
	 
	writel(ch,uart_base+DR);
}
static char uart_rx(void)
{
	while(readl(uart_base+FR) & FR_RX)
		cpu_relax();
	return readl(uart_base+DR) & 0xff;

}
static void uart_tx_string(char *str)
{
	while(*str)
		uart_tx(*str);
}
static void led_blink(void)
{
	uart_tx('a');
	char ch=uart_rx();
	if(ch=='a')
		gpio_set_value(GPIO_LED,1);
	else
		gpio_set_value(GPIO_LED,0);
pr_info("Come in led_blink is led is blinking ...?\n");


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
	int reg=readl(gpio_base+GPFSEL0);
	reg &= ~((7<<3) | (7<<0));
	reg |= (3<<3)|(3<<0);
	
	writel(reg,gpio_base+GPFSEL0);
	
	writel(0x0,uart_base+CR);
	/* baud rate */
		//9600
	writel(312,uart_base+IBRD);
	writel(32,uart_base+FBRD);
	/* 8 data bits and fifo enabled */
	writel(0x70,uart_base+LCRH);
	/*  uart enable */
	writel((1<<0)|(1<<8)|(1<<9),uart_base+CR);

	/*	LED	*/
	
	int f=readl(gpio_base+GPFSEL2);
	f &= ~(7<<3);
	f |= (1<<3); // gpio output direction
	
	uart_tx_string("\n\r Uart initialised successfully\n");
	
	led_blink();
	return 0;
}

static void __exit my_exit(void)
{
	pr_info("UART exit\n");
	gpio_set_value(GPIO_LED,0);
	iounmap(uart_base);
	iounmap(gpio_base);
	
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("UART2");

