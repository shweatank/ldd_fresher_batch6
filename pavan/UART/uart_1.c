#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>

#define UART2_BASE 0xFE201400 //UART2 PL01 1 base (Pi 4 typical)

static void __iomem *uart2_base;

/*Register offset*/
#define UART_DR 0x00
#define UART_FR 0x18
#define UART_IBRD 0x24
#define UART_FBRD 0x28
#define UART_LCRH 0x2C
#define UART_CR 0x30

/*Flag bits*/
#define UART_FR_TXFF (1 << 5) //TX FIFO full
#define UART_FR_RXFE (1 << 4) //RX FIFO empty

/*write one byte*/
static void uart2_write_char(char c)
{
	/*Wait until TX FIFO not full*/
	while(readl(uart2_base + UART_FR) & UART_FR_TXFF)
		cpu_relax();

	writel(c,uart2_base + UART_DR);
}

/*Read one byte*/
static char uart2_read_char(void)
{
	/*wait until data available*/
	while(readl(uart2_base + UART_FR) & UART_FR_RXFE)
		cpu_relax();

	return (char)(readl(uart2_base + UART_DR) & 0xFF);
}

/*Write string*/
static void uart2_write_string(const char *str)
{
	while(*str)
		uart2_write_char(*str++);
}

/*init UART2*/
static int __init uart2_init(void)
{
	printk(KERN_INFO "UART2 drive init\n");

	uart2_base = ioremap(UART2_BASE,0x1000);
	if(!uart2_base)
	{
		printk(KERN_ERR "ioremap failed\n");
		return -ENOMEM;
	}

	/*Disable UART*/
	writel(0x0,uart2_base + UART_CR);
	/*config baud rate*/
	writel(1,uart2_base+UART_IBRD);
	writel(40,uart2_base + UART_FBRD);

	/*8N1, enble FIFO*/
	writel((1 << 4) | (3 << 5), uart2_base + UART_LCRH);

	/*Enable UART, TX, RX*/
	writel((1 << 0) | (1 << 8) | (1 << 9), uart2_base + UART_CR);

	uart2_write_string("UART2 Ready!\n");

	return 0;
}

/*Exit*/
static void __exit uart2_exit(void)
{
	uart2_write_string("UART2 Exit\n");
	if(uart2_base)
		iounmap(uart2_base);

	printk(KERN_INFO "UART2 driver exit\n");

}

module_init(uart2_init);
module_exit(uart2_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PAVAN");
MODULE_DESCRIPTION("Simple UART2 ioremap driver for Raspberry Pi 4");

