#include<linux/module.h>
#include<linux/init.h>
#include<linux/gpio.h>
#include<linux/kernel.h>
#include<linux/io.h>
#include<linux/delay.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PRUDHVI");
MODULE_DESCRIPTION("UART DRIVER PROGRAM");

#define uart_base_phys 0XFE201000
#define uart_size 1000

#define UART_DR 0X00
#define UART_FR 0X18
#define UART_IBRD 0x24
#define UART_FBRD 0x28
#define UART_LCRH 0x2C
#define UART_CR  0x30

#define TXFF 1<<5
#define RXFE 1<<4

#define UARTEN 1<<0
#define TXE 1<<8
#define RXE 1<<9

static void __iomem *uart_base;
static void uart_init(void)
{
    // Disable UART
    writel(0x0, uart_base + UART_CR);

    // Baud rate: 9600 (assuming 48MHz clock)
    writel(312, uart_base + UART_IBRD);
    writel(32,  uart_base + UART_FBRD);

    // 8-bit, no parity, 1 stop bit
    writel((3 << 5), uart_base + UART_LCRH);

    // Enable UART, TX, RX
    writel(UARTEN | TXE | RXE, uart_base + UART_CR);

    pr_info("UART initialized\n");
}


// ---------------- SEND CHAR ----------------
static void uart_send_char(char c)
{
    // Wait until TX FIFO not full
    while (readl(uart_base + UART_FR) & TXFF);

    writel(c, uart_base + UART_DR);
}

// ---------------- RECEIVE CHAR ----------------
static char uart_recv_char(void)
{
    // Wait until RX FIFO not empty
    while (readl(uart_base + UART_FR) & RXFE);

    return (char)readl(uart_base + UART_DR);
}

// ---------------- SEND STRING ----------------
static void uart_send_string(const char *str)
{
    while (*str) {
        uart_send_char(*str++);
    }
}
static int __init my_init(void)
{

	uart_base=ioremap(uart_base_phys,uart_size);
	if(!uart_base)
	{
		pr_err("Failed to map UART\n");
		return -ENOMEM;
	}
	uart_init();

	// Send test message
	uart_send_string("Hello from Kernel UART!\n");
//	pr_info("uart:recevird =%c\n",uart_recv_char());
	return 0;
}

static void __exit my_exit(void)
{
pr_info("UART Driver Exit\n");

    // Disable UART
    writel(0x0, uart_base + UART_CR);

    iounmap(uart_base);


}

module_init(my_init);
module_exit(my_exit);
