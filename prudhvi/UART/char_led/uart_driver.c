#include<linux/module.h>
#include<linux/init.h>
#include<linux/gpio.h>
#include<linux/kernel.h>
#include<linux/io.h>
#include<linux/delay.h>
#include<linux/gpio.h>
#include<linux/workqueue.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PRUDHVI");
MODULE_DESCRIPTION("UART DRIVER PROGRAM");
#define DEVICE_NAME "uart_led_blink"
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


#define GPIO_LED (512+21)
static void __iomem *uart_base;
static struct work_struct my_work;
static void uart_init(void)
{
	// Disable UART
	writel(0x0, uart_base + UART_CR);

	// Baud rate: 9600 (assuming 48MHz clock)
	writel(312, uart_base + UART_IBRD);
	writel(32,  uart_base + UART_FBRD);

	// 8-bit, no parity, 1 stop bit
	writel((3 << 5)|(1<<4), uart_base + UART_LCRH);

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
	char c;
	pr_info("Waiting for input...\n");
	while (readl(uart_base + UART_FR) & RXFE)
	{
		cpu_relax();
		udelay(10);
	}

	c= readl(uart_base + UART_DR) & 0xFF;
	pr_info("Received: %c\n", c);
	return c;
}

// ---------------- SEND STRING ----------------
static void uart_send_string(const char *str)
{
	while (*str) {
		uart_send_char(*str++);
	}
}
static void my_work_handler(struct work_struct *work)
{
	char s;
	for(int i=0;i<10;i++)
	{
		uart_send_string("Choose your option...\n\r");
		uart_send_string("1)ON\n\r");
		uart_send_string("2)OFF\n\r");
		uart_send_string("3)Exit\n\r");

		s=uart_recv_char();
		if(s=='1')
		{

			//		gpio_set_value(GPIO_LED,1);
			gpio_set_value_cansleep(GPIO_LED, 1);
		}
		else if(s=='2')
		{
			//	gpio_set_value(GPIO_LED, 0);  // Turn off LED
			gpio_set_value_cansleep(GPIO_LED, 0);
		}
		else if(s=='3')
		{
			uart_send_string("bye bye!\n");
			return;
		}
		else
		{

			uart_send_string("wrong option selected!\n");

		}

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

	// Request the GPIO
	if (!gpio_is_valid(GPIO_LED)) {
		iounmap(uart_base);
		pr_err("LED: Invalid GPIO %d\n", GPIO_LED);
		return -ENODEV;
	}
	if(gpio_request(GPIO_LED,DEVICE_NAME))
	{
		iounmap(uart_base);
		pr_err("LED: Failed to request GPIO %d\n", GPIO_LED);
		return -EBUSY;

	}
	gpio_direction_output(GPIO_LED, 0); 
	// Send test message
	uart_send_string("Hello from Kernel UART!\n");
	pr_info("workqueue module loaded\n");
	//initialize work
	INIT_WORK(&my_work,my_work_handler);

	//schedule work
	pr_info("workqueue:scheduling  woek\n");
	schedule_work(&my_work);

	return 0;
}

static void __exit my_exit(void)
{
	pr_info("UART Driver Exit\n");

	// Disable UART
	writel(0x0, uart_base + UART_CR);
	iounmap(uart_base);
	pr_info("Workqueue module exiting\n");
	//ensure work is completed before exit
	flush_work(&my_work);
	pr_info("Workqueue module unloaded\n");

	gpio_set_value(GPIO_LED, 0);  // Turn off LED
	gpio_free(GPIO_LED);

}

module_init(my_init);
module_exit(my_exit);
