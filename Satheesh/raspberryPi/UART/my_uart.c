
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>


#define UART_BASE 0xFe201000

#define DR 0x00
#define FR 0x04
#define IBRD 0x24
#define FBRD 0x28
#define LCRH 0x2c
#define CR 0x30
#define ICR 0x44

#define GPFSEL1 0x04
#define GPFSET0 0x1c
#define GPFCLR0 0x28

#define GPIO_LED (512+17)

#define FR_TXFF (1<<5)
#define FR_RXFE (1<<4)



#define DEVICE_NAME "uart_davice"
/* GPIO0 GPIO1 */
static void __iomem* uart_base;
static void __iomem* gpio_base;
char ch='A';
static void uart_putc(char);
static char uart_getc(void);
static void uart_blink(void)
{
	uart_putc(ch);
	pr_info("Txed byte %c\n",ch);
	udelay(100);
	char x=uart_getc();
	if(ch==0)
	{
		gpio_set_value(GPIO_LED,1);
		pr_info("LED ON\n");

	}
else
	gpio_set_value(GPIO_LED,0);

}

static void uart_putc(char ch)
{
	while(readl(uart_base+FR) & FR_TXFF)
		cpu_relax();
	writel(ch,uart_base+DR);
}

static char uart_getc(void)
{
	while(readl(uart_base+FR) & FR_RXFE)
		cpu_relax();
	return readl(uart_base+DR)&& 0xff;
}


static void __init uart_init(void)
{
	pr_info("uart driver\n");
	uart_base=ioremap(UART_BASE,0x1000);
	if(!uart_base)
	{
		pr_err("IOREMAP: Failed to remap\n");
		return -ENOMEM;
	}
	gpio_base=ioremep(GPIO_BASE,0x1000);
	if(!gpio_base)
	{
		pr_err("GPIO ioremap failed\n");
		return -ENOMEM;
	}
	int val=readl(gpio_base+GPFSEL1);
	val &= ~((7<<12)|(7<<15));
	val |=(4<<12) | (4<<15);
	writel(val,gpio_base+GPFSEL1);
	int x=readl(uart_base+GPFSEL1);
	x &= (1<<21);
	x |= (1<<21);
	writel(x,gpio_base+GPFSEL1);
	writel(0x0,uart_base+CR);
	
	writel(0x7ff,uart_base+ICR);
	

	writel(26,uart_base_IBRD);
	writel(3,uart_base+UART_FBRD);

	writel((0<<4)| (1<<8) | (1<<9),uart_base+CR);

	uart_puts("\n\r UART ioremap is ready\n");
	pr_info("Right before uart blink\n");
	uart_blink();
	return 0;
}
static void __exit uart_exit(void)
{
	pr_info("Uart exit func\n");
	gpio_set_value(GPIO_LED,0);
	writel(0x0,uart_base+CR);
	if(uart_base)	iounmap(uart_base);
	pr_info("Uart ioremap unloaded\n");
	pr_info("UART ioremap driver unloaded\n");
}
module_init(uart_init);
module_exit(uart_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("BCM2711 Uart driver\n");

















