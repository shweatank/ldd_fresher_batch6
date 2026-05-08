// uart_irq_driver.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#define UART_BASE   0xFE201400   // Example base address
#define UART_DR     (UART_BASE + 0x00)
#define UART_IMSC   (UART_BASE + 0x38)
#define UART_MIS    (UART_BASE + 0x40)
#define UART_ICR    (UART_BASE + 0x44)
#define UART_IBRD 0x24
#define UART_FBRD 0x28
#define UART_LCRH 0x2C
#define UART_CR  0x30
#define RX_INT  (1 << 4)
#define TX_INT  (1 << 5)
#define UARTEN 1<<0
#define TXE 1<<8
#define RXE 1<<9
#define UART_IRQ  38   // Example IRQ number

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

/* Interrupt Handler */
static irqreturn_t uart_irq_handler(int irq, void *dev_id)
{
    u32 status;

    status = readl(uart_base + 0x40); // UART_MIS

    // 📥 RX interrupt
    if (status & RX_INT) {
        char data = readl(uart_base + 0x00); // UART_DR
        printk(KERN_INFO "UART RX: %c\n", data);

        writel(RX_INT, uart_base + 0x44); // Clear RX interrupt
    }

    // 📤 TX interrupt
    if (status & TX_INT) {
        writel('A', uart_base + 0x00); // Send data
        printk(KERN_INFO "UART TX triggered\n");

        writel(TX_INT, uart_base + 0x44); // Clear TX interrupt
    }

    return IRQ_HANDLED;
}

/* Init function */
static int __init uart_driver_init(void)
{
    printk(KERN_INFO "UART IRQ Driver Init\n");

    uart_base = ioremap(UART_BASE, 0x100);
    if (!uart_base)
        return -ENOMEM;
uart_init();
    // Enable RX & TX interrupts
    writel(RX_INT | TX_INT, uart_base + 0x38); // UART_IMSC

    // Register IRQ
    if (request_irq(UART_IRQ, uart_irq_handler, 0, "uart_irq", NULL)) {
        printk(KERN_ERR "IRQ request failed\n");
        return -1;
    }

    return 0;
}

/* Exit function */
static void __exit uart_driver_exit(void)
{
    free_irq(UART_IRQ, NULL);
    iounmap(uart_base);

    printk(KERN_INFO "UART IRQ Driver Exit\n");
}

module_init(uart_driver_init);
module_exit(uart_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Simple UART Interrupt Driver");
