#include<linux/init.h>
#include<linux/module.h>
#include<linux/io.h>
#include<linux/delay.h>

#define UART2_BASE 0xFe201400

static void __iomem *uart2_base;

#define UART2_DR 0x00
#define UART2_FR 0x18
#define UART2_IBRD 0x24
#define UART2_FBRD 0x28
#define UART2_LCRH 0x2c
#define UART2_CR 0x30
#define UART2_FR_TXFF (1 << 5)   // TX FIFO full
#define UART2_FR_RXFE (1 << 4)   // RX FIFO empty

//#define UART_FR_TXFF (1 << 5)   // TX FIFO full
//#define UART_FR_RXFE (1 << 4)   // RX FIFO empty
static void uart2_write_char(char c)
{
	while(readl(uart2_base+UART2_FR) &  UART2_FR_TXFF)
		cpu_relax();
	writel(c,uart2_base+UART2_DR);
}
static char uart2_read_char(void)
{
	 while (readl(uart2_base + UART2_FR) & UART2_FR_RXFE)
        	cpu_relax();
	return (char )(readl(uart2_base+UART2_DR) & 0xff);
}
static void uart2_write_string(const char *str)
{
    while (*str)
        uart2_write_char(*str++);
}
static void uart2_read_string(void)
{
	while(1)
	{
		char ch;
		ch=uart2_read_char();
		printk("Received Char: %c",ch);
	}
}
static int __init uart2_init(void)
{
	pr_info("uart2 init\n");
	
	uart2_base=ioremap(UART2_BASE,0x1000);
	if(!uart2_base)
	{
		pr_err("IOREMAP Failed\n");
		return -ENOMEM;
	}
	//Disable Uart
	writel(0x0,uart2_base+UART2_CR);
	//Setting Baud rate 
	writel(26, uart2_base + UART2_IBRD);
	writel(3, uart2_base + UART2_FBRD);
	
	writel((1<4) |(3<<5),uart2_base+UART2_LCRH);
	//Enable uart tx and rx 
	writel((1<<0) | (1<<8) |(1<<9), uart2_base+UART2_CR);
		
	uart2_read_string();	
	uart2_write_string("Hi Satheesh\n");
	return 0;
}
static void __exit uart2_exit(void)
{
	if(uart2_base)
		iounmap(uart2_base);
	uart2_write_string("In Exit\n");
}

module_init(uart2_init);
module_exit(uart2_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("Simple uart driver");






