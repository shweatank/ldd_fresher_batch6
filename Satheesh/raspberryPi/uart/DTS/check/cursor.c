#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/printk.h>
#define DR   0x00
#define FR   0x18
#define IBRD 0x24
#define FBRD 0x28
#define LCRH 0x2c
#define CR   0x30
#define IMSC 0x38
#define MIS  0x40
#define ICR  0x44

/* PL011 FR bits */
#define FR_RXFE (1 << 4) /* RX FIFO empty */
#define FR_TXFF (1 << 5) /* TX FIFO full */

/* PL011 interrupt bits (MIS/IMSC/ICR) */
#define INT_RX   (1 << 4) /* Receive interrupt */
#define INT_TX   (1 << 5) /* Transmit interrupt */
#define INT_RT   (1 << 6) /* Receive timeout interrupt */
static void __iomem *uart_base;

static char buffer[64];
static int index;
static const char tx_buf[] = "loopback irq test: TX->RX\n";
static int tx_len;
static int tx_index;
static int rx_count;
static bool tx_active;

static void uart_kick_tx(void)
{
	tx_index = 0;
	tx_active = true;
	/* Disable and clear pending interrupts before enabling. */
	writel(0, uart_base + IMSC);
	writel(0x7FF, uart_base + ICR);
	/* Enable RX, RX timeout and TX interrupts */
	writel(INT_RX | INT_RT | INT_TX, uart_base + IMSC);
	pr_info("UART loopback TX started, tx_len=%d\n", tx_len);
}
/* ---------- ISR ---------- */
static irqreturn_t uart_irq_handler(int irq, void *dev_id)
{
	unsigned int status;
	bool handled = false;

	status = readl(uart_base + MIS);
	pr_info("UART IRQ handler entered: status=0x%x, FR=0x%x\n",
		status, readl(uart_base + FR));

	/* RX (and receive-timeout) handling */
	if (status & (INT_RX | INT_RT)) {
		handled = true;

		while (!(readl(uart_base + FR) & FR_RXFE)) {
			char ch = readl(uart_base + DR) & 0xFF;

			buffer[index++] = ch;
			rx_count++;
			if (index >= sizeof(buffer) - 1) {
				/* Prevent overflow; keep last byte as '\0'. */
				index = sizeof(buffer) - 2;
				break;
			}
		}
		buffer[index] = '\0';

		/*
		 * Clear RX/RT interrupts *after* draining FIFO,
		 * otherwise the interrupt can immediately retrigger.
		 */
		writel(INT_RX | INT_RT, uart_base + ICR);

		/*
		 * Loopback verification: once we have received at least the TX
		 * string length, check for the expected substring.
		 */
		if (index >= tx_len) {
			if (strnstr(buffer, tx_buf, index))
				pr_info("UART loopback PASS (rx=%d bytes)\n", rx_count);
			else
				pr_warn("UART loopback FAIL (rx=%d bytes, got='%s')\n",
					rx_count, buffer);

			/* Stop RX interrupts after verdict to avoid log spam. */
			writel(readl(uart_base + IMSC) & ~(INT_RX | INT_RT),
			       uart_base + IMSC);
		}
	}

	/* TX handling */
	if ((status & INT_TX) && tx_active) {
		handled = true;

		while (!(readl(uart_base + FR) & FR_TXFF)) {
			if (tx_index >= tx_len) {
				tx_active = false;
				/* Disable TX interrupt when nothing to send */
				writel(readl(uart_base + IMSC) & ~INT_TX,
				       uart_base + IMSC);
				pr_info("UART TX completed (%d bytes)\n", tx_len);
				break;
			}
			writel(tx_buf[tx_index++], uart_base + DR);
		}

		/* Clear TX interrupt */
		writel(INT_TX, uart_base + ICR);
	}

	if (!handled)
		pr_warn_ratelimited("UART IRQ handler: no recognized status bits 0x%x\n", status);

	return handled ? IRQ_HANDLED : IRQ_NONE;
}

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
                           IRQF_SHARED,
                           "my_uart2",
                           pdev);
    if (ret) {
        dev_err(&pdev->dev, "IRQ request failed: %d\n", ret);
        return ret;
    }

    /* UART Configuration */

    writel(0x0, uart_base + CR);     // disable UART
    writel(0, uart_base + IMSC);
    writel(0x7FF, uart_base + ICR);

    /* baud rate (example: 115200 for 48MHz clock) */
    writel(26, uart_base + IBRD);
    writel(3,  uart_base + FBRD);



    /* 8-bit, FIFO enable */
    writel(0x70, uart_base + LCRH);
    /* Clear interrupts */
    writel(0x7FF, uart_base + ICR);

    /* Enable UART, RX, TX */
    writel((1<<0) | (1<<8) | (1<<9), uart_base + CR);

	tx_len = strlen(tx_buf);
	index = 0;
	rx_count = 0;
	memset(buffer, 0, sizeof(buffer));

	/* Start short interrupt-driven loopback TX */
	uart_kick_tx();

	dev_info(&pdev->dev, "UART initialized, IRQ=%d\n", irq);
	
	pr_info("Probe execution complete\n");

    return 0;
}

/* ---------- Remove ---------- */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
static void my_remove(struct platform_device *pdev)
{
    writel(0, uart_base + IMSC);
    writel(0x7FF, uart_base + ICR);
    writel(0x0, uart_base + CR);
    pr_info("UART driver removed\n");
}
#else
static void my_remove(struct platform_device *pdev)
{
    writel(0, uart_base + IMSC);
    writel(0x7FF, uart_base + ICR);
    writel(0x0, uart_base + CR);
    pr_info("UART driver removed\n");
}
#endif

/* ---------- Device Match ---------- */
static const struct of_device_id my_of_match[] = {
    { .compatible = "my_uart2" },
    { }
};
MODULE_DEVICE_TABLE(of, my_of_match);

/* ---------- Platform Driver ---------- */
static struct platform_driver my_driver = {
    .probe  = my_probe,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
    .remove = my_remove,
#else
    .remove = my_remove,
#endif
    .driver = {
        .name = "my_uart2",
        .of_match_table = my_of_match,
    },
};

module_platform_driver(my_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("Simple UART2 Driver with DT + IRQ");


