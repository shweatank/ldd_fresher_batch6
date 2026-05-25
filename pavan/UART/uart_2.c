#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#define UART0_BASE_PHYS   0xFE201000
#define UART0_SIZE         0x90
#define GPIO_BASE 0xFE200000
#define GSIZE 0xB4
#define GPFSEL1 	0x04
#define GPFSET0		0x1c	
#define GPFCLR0 	0x28
#define GPIO_LED (512+17)


/* UART Registers */
#define UART_DR     0x00
#define UART_FR     0x18
#define UART_IBRD   0x24
#define UART_FBRD   0x28
#define UART_LCRH   0x2C
#define UART_CR     0x30
#define UART_IMSC   0x38
#define UART_ICR    0x44

/* FR bits */
#define FR_TXFF     (1 << 5)
#define FR_RXFE     (1 << 4)

static void __iomem *uart_base;
static void __iomem *gp_base;

char ch='A';
static void uart_putc(char);
static char uart_getc(void);

static void uart_blink(void){
	uart_putc(ch);
	pr_info("txed byte is %c\n",ch);
	pr_info("txed a byte\n");
	udelay(100);
	char x=uart_getc();
	if(x==ch){
		gpio_set_value(GPIO_LED,1);
		pr_info("%c\n",x);
		pr_info("led on\n");
	}	
	else
		gpio_set_value(GPIO_LED,0);
}

/* Polling TX */
static void uart_putc(char c)
{
    while (readl(uart_base + UART_FR) & FR_TXFF)
        cpu_relax();

    writel(c, uart_base + UART_DR);
}

/* Polling RX */
static char uart_getc(void)
{
    while (readl(uart_base + UART_FR) & FR_RXFE)
        cpu_relax();

    return readl(uart_base + UART_DR) & 0xFF;
}

static void uart_puts(const char *s)
{
    while (*s)
        uart_putc(*s++);
}

static int __init uart_init(void)
{
    pr_info("UART ioremap driver init\n");

    uart_base = ioremap(UART0_BASE_PHYS, UART0_SIZE);
    if (!uart_base) {
        pr_err("UART ioremap failed\n");
        return -ENOMEM;
    }
    gp_base = ioremap(GPIO_BASE, GSIZE);
    if(!gp_base){
	    pr_err("GPIO ioremap failed\n");
	    return -ENOMEM;
    }
	
    int val = readl(gp_base + GPFSEL1);
    val &= ~((7<<12)|(7<<15));
    val|=(4<<12)|(4<<15);
    writel(val, gp_base+GPFSEL1);
    int x=readl(gp_base + GPFSEL1);
    x&=~(1<<21);
    x|=(1<<21);
	writel(x,gp_base+GPFSEL1);
	//set_direction_output(GPIO_LED,0);
    /* Disable UART */
    writel(0x0, uart_base + UART_CR);

    /* Clear interrupts */
    writel(0x7FF, uart_base + UART_ICR);

    /*
     * Baud rate = 115200
     * UARTCLK = 48MHz
     * Divider = 48,000,000 / (16 * 115200) = 26.0416
     */
    writel(26, uart_base + UART_IBRD);
    writel(3,  uart_base + UART_FBRD);

    /* 8N1, FIFO enabled */
    writel((0 << 4) | (1 << 5) | (1 << 6),
           uart_base + UART_LCRH);

    /* Enable UART, TX, RX */
    writel((1 << 0) | (1 << 8) | (1 << 9),
           uart_base + UART_CR);

    uart_puts("\n\rUART ioremap driver active\r\n");
	pr_info("right before uart blink\n");	
    uart_blink();
	pr_info("uart blink called\n");	

    /* Echo test 
    uart_puts("Type a character...\r\n");

    {
        char c = uart_getc();
        uart_puts("You typed: ");
        uart_putc(c);
        uart_puts("\r\n");
    }*/
	pr_info("returning from init\n");
    return 0;
}

static void __exit uart_exit(void)
{
    //uart_puts("UART driver exit\r\n");

	pr_info("uart exit func\n");
	gpio_set_value(GPIO_LED,0);
    writel(0x0, uart_base + UART_CR);

    if (uart_base)
        iounmap(uart_base);

    pr_info("UART ioremap driver unloaded\n");
}

module_init(uart_init);
module_exit(uart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prashant");
MODULE_DESCRIPTION("BCM2711 UART driver using ioremap only");
