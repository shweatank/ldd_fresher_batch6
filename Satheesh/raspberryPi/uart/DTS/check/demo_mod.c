#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/delay.h>
#define DR   0x00
#define FR   0x18
#define IBRD 0x24
#define FBRD 0x28
#define LCRH 0x2c
#define CR   0x30
#define IMSC 0x38
#define MIS  0x40
#define ICR  0x44

#define FR_RX (1<<4)
#define FR_TX (1<<5)
static void __iomem *uart_base;

static char buffer[64];
static int index;
static char tx_buf[]="Hello im in tx buffer\n";
/* ---------- ISR ---------- */
static irqreturn_t uart_irq_handler(int irq, void *dev_id)
{
	pr_info("Hello Im in irq handler\n");
    unsigned int status = readl(uart_base + MIS);

    if (status & (1 << 4)) {   // RX interrupt

	pr_info("In rx handler babu\n");
        /* Drain FIFO completely */
        while (!(readl(uart_base + FR) & FR_RX)) {
            char ch = readl(uart_base + DR) & 0xFF;

            buffer[index++] = ch;
            if (index >= sizeof(buffer) - 1)
                index = 0;
        }

        buffer[index] = '\0';

        pr_info("UART RX: %s\n", buffer);

        /* Clear RX interrupt */
        writel((1 << 4)|(1<<6), uart_base + ICR);
    }
 if (status & (1 << 5))   // TX interrupt
{
    static int tx_index = 0;
    /* Fill TX FIFO until full */
    while (!(readl(uart_base + FR) & FR_TX))
    {
        /* All data sent */
        if (tx_buf[tx_index] == '\0')
        {
            tx_index = 0;

            /* Disable TX interrupt */
            writel(readl(uart_base + IMSC) & ~(1 << 5),
                   uart_base + IMSC);

            pr_info("TX completed\n");

            break;
        }

        /* Put byte into TX FIFO */
        writel(tx_buf[tx_index++], uart_base + DR);
    }

    /* Clear TX interrupt */
    writel((1 << 5), uart_base + ICR);
}


    return IRQ_HANDLED;
}

/*static void check(void)
{
	writel('A',uart_base+DR);
	char ch=readl(uart_base_DR);
	if(ch=='A')
		pr_info("loop back working properly\n");
	else
		pr_info("Loopback not working\n");
}*/
static void check(void)
{
    char ch;

    pr_info("Starting UART polling loopback test\n");
 char c[]="aaaaaaaaaaaaaaaajjjjjjjjjjjjjjjjsdddddddddddsssssssscccccccccccccccccssssssssssssssssffffffffffffffeeeeeeeeeekkkkkkkkkkkkksdfnskdjfhks";
char *ptr=c;
	while(*ptr){
    /* Wait until TX FIFO has space */
    while (readl(uart_base + FR) & FR_TX);

    /* Send byte */
    writel(*ptr++, uart_base + DR);
}
    pr_info("Byte transmitted\n");

    /* Wait until RX FIFO gets data */
//    while (readl(uart_base + FR) & FR_RX);

    /* Read received byte */
  //  ch = readl(uart_base + DR) & 0xFF;

    //if (ch == 'A')
      //  pr_info("Loopback working properly\n");
    //else
      //  pr_info("Wrong byte received: %c\n", ch);

}

/* ---------- Probe ---------- */
static int my_probe(struct platform_device *pdev)
{
    struct resource *res;
    int irq;
    int ret;

    pr_info("UART driver probe\n");

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

    /* baud rate (example: 115200 for 48MHz clock) */
    writel(26, uart_base + IBRD);
    writel(3,  uart_base + FBRD);



    /* 8-bit, FIFO enable */
    writel(0x70, uart_base + LCRH);
unsigned char tmp=readl(uart_base+LCRH);
	tmp &=~(1<<4);
	writel(tmp,uart_base+LCRH);
    /* Clear interrupts */
    writel(0x7FF, uart_base + ICR);

    /* Enable RX interrupt */
    writel(1 << 4, uart_base + IMSC);

    /* Enable UART, RX, TX */
    writel((1<<0) | (1<<8) | (1<<9), uart_base + CR);

    /* Enable RX TX interrupt */
    writel((1 << 4)|FR_TX , uart_base + IMSC);
    
//	writel(*tx_buf,uart_base+DR);
	dev_info(&pdev->dev, "UART initialized, IRQ=%d\n", irq);
	
	pr_info("Probe execution complete\n");
	check();

    return 0;
}

/* ---------- Remove ---------- */
static void my_remove(struct platform_device *pdev)
{
    writel(0x0, uart_base + CR);
    pr_info("UART driver removed\n");

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
    //.remove = my_remove,
    .driver = {
        .name = "my_uart2",
        .of_match_table = my_of_match,
    },
};

module_platform_driver(my_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("Simple UART2 Driver with DT + IRQ");

