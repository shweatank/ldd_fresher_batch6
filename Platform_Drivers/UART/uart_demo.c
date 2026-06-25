
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>

#define UART_DR     0x00
#define UART_FR     0x18

#define TXFF        (1 << 5)

static void __iomem *uart_base;

static void uart_send_char(char ch)
{
    while (readl(uart_base + UART_FR) & TXFF)
        cpu_relax();

    writel(ch, uart_base + UART_DR);
}

static void uart_send_string(const char *str)
{
    while (*str)
        uart_send_char(*str++);
}

static int uart_demo_probe(struct platform_device *pdev)
{
    struct resource *res;

    pr_info("uart_demo: probe called\n");

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res)
        return -ENODEV;

    uart_base = ioremap(res->start, resource_size(res));
if (!uart_base)
    return -ENOMEM;

    uart_send_string("\r\n");
    uart_send_string("====================================\r\n");
    uart_send_string(" UART0 PLATFORM DRIVER LOADED\r\n");
    uart_send_string(" Hello From Raspberry Pi UART0\r\n");
    uart_send_string("====================================\r\n");

    pr_info("uart_demo: data transmitted\n");

    return 0;
}

static void uart_demo_remove(struct platform_device *pdev)
{
	if (uart_base)
        iounmap(uart_base);
    pr_info("uart_demo: removed\n");
}

static const struct of_device_id uart_demo_match[] = {
    {
        .compatible = "vaishnavi,uart-demo",
    },
    {},
};

MODULE_DEVICE_TABLE(of, uart_demo_match);

static struct platform_driver uart_demo_driver = {
    .probe  = uart_demo_probe,
    .remove = uart_demo_remove,
    .driver = {
        .name = "uart_demo",
        .of_match_table = uart_demo_match,
    },
};

module_platform_driver(uart_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vaishnavi");
MODULE_DESCRIPTION("UART0 Platform Driver");

