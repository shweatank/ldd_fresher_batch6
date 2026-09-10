#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include<linux/delay.h>

#define DR   0x00
#define FR   0x18
#define IBRD 0x24
#define FBRD 0x28
#define LCRH 0x2c
#define CR   0x30
#define IMSC 0x38
#define MIS  0x40
#define ICR  0x44

#define GPFSEL0 (0x00)

#define FR_RX (1<<4)
#define FR_TX (1<<5)

#define GPIO_BASE 0xfe200000
static void __iomem *uart_base;
static void __iomem *gpio_base;
static char buffer[64];
static int index;

/* ---------- ISR ---------- */
static void uart_tx(char ch)
{
    /* Wait until TX FIFO is not full */
    while ((readl(uart_base + FR) )& (1 << 5))
        cpu_relax();

    writel(ch, uart_base + DR);
}
static void uart_tx_string(char *str)
{
	while(*str)
		uart_tx(*str++);
}
static irqreturn_t uart_irq_handler(int irq, void *dev_id)
{
    unsigned int status = readl(uart_base + MIS);

pr_info("In isr\n");  
  if (status & (1 << 4)) {   // RX interrupt

	char ch=readl(uart_base+DR);
/*	if(index<63){
		buffer[index++]=ch;
		buffer[index]='\0';
	}
        if(index>9)
		pr_info("Data in buffer:%s\n",buffer);
	 Clear RX interrupt */

	//pr_info("Handler triggered\n");
	pr_info("received:%c\n",ch);
        writel(1 << 4, uart_base + ICR);
    }
	

    return IRQ_HANDLED;
}
static void uart_rx(void)
{
while (1) {
    if (!(readl(uart_base + FR) & (1 << 4))) {
        char ch = readl(uart_base + DR);
        pr_info("POLL RX: %c\n", ch);
    }
}
}

/* ---------- Probe ---------- */
static int my_probe(struct platform_device *pdev)
{
    struct resource *res;
    int irq;
    int ret;


    pr_info("UART driver probe\n");
	gpio_base=ioremap(GPIO_BASE,0x90);
	if(IS_ERR(gpio_base))
		return PTR_ERR(gpio_base);
	writel(0x1B,gpio_base+GPFSEL0);
    /* Get memory from DT */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    uart_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(uart_base))
        return PTR_ERR(uart_base);

    /* Get IRQ from DT */
    irq = platform_get_irq(pdev, 0);
    if (irq < 0)
        return irq;

    /* Request IRQ */
    ret = devm_request_irq(&pdev->dev, irq,
                           uart_irq_handler,
                           0,
                           "my_uart2",
                           NULL);
    if (ret) {
        dev_err(&pdev->dev, "IRQ request failed\n");
        return ret;
    }

    /* UART Configuration */

    writel(0x0, uart_base + CR);     // disable UART

    /* baud rate (example: 9600 for 48MHz clock) */
    writel(26, uart_base + IBRD);
    writel(3,  uart_base + FBRD);

    /* 8-bit, FIFO enable */
	unsigned char ch=readl(uart_base+LCRH);
	ch &= ~(1<<4);
	writel(ch,uart_base+LCRH);
	
   writel(0x60, uart_base + LCRH);

    /* Clear interrupts */
    writel(0x7FFF, uart_base + ICR);
	//writel((1<<4) | (1<<6), uart_base+0x38);
	writel((1<<4), uart_base+0x38);
	
    /* Enable RX interrupt */
  //  writel(1 << 4, uart_base + IMSC);

    /* Enable UART, RX, TX */
    writel((1<<0) | (1<<8) | (1<<9), uart_base + CR);

    dev_info(&pdev->dev, "UART initialized, IRQ=%d\n", irq);

//	uart_tx_string("Hellooo Im working\n");
	uart_tx_string("Satheesh Kumar");

return 0;
}

/* ---------- Remove ---------- */
static int  my_remove(struct platform_device *pdev)
{
    writel(0x0, uart_base + CR);

    pr_info("UART driver removed\n");
	return 0;
}

/* ---------- Device Match ---------- */
static const struct of_device_id my_of_match[] = {
    { .compatible = "my_uart2" },
    { }
};
MODULE_DEVICE_TABLE(of, my_of_match);

/* ---------- Platform Driver ---------- */
static struct platform_driver my_driver = {
    .probe  = my_probe,
    .remove = my_remove,
    .driver = {
        .name = "my-uart2",
        .of_match_table = my_of_match,
    },
};

module_platform_driver(my_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("Simple UART2 Driver with DT + IRQ");
