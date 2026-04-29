#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>  // Required for IRQ handling

#define UART2_BASE 0xFE201400
#define GPIO_BASE 0xFE200000

#define DR 0x00
#define FR 0x04
#define IBRD 0x24
#define FBRD 0x28
#define LCRH 0x2c
#define CR 0x30
#define ICR 0x44
#define IMSC 0x38   // Interrupt Mask Set/Clear Register

#define GPFSEL0 0x00
#define GPFSEL2 0x08
#define GPFSET0 0x1c
#define GPFCLR0 0x28

#define GPIO_LED (512+21)

#define FR_TX (1<<5)
#define FR_RX (1<<4)

// UART base and GPIO base addresses
static void __iomem* uart_base;
static void __iomem* gpio_base;

// RX Interrupt Handler
static void uart_rx_interrupt_handler(void)
{
    char ch;

    // Check if data is available in the RX FIFO (FR_RX bit in FR register is low when data is ready)
    while (!(readl(uart_base + FR) & FR_RX)) {
        // Read the received data from the RX FIFO
        ch = readl(uart_base + DR) & 0xff;
        // Process received data (you can customize this)
        pr_info("Received data: %c\n", ch);
        if (ch == 'a') {
            gpio_set_value(GPIO_LED, 1);
        } else {
            gpio_set_value(GPIO_LED, 0);
        }
    }
}

// Function to enable RX interrupt in UART IMSC register
static void uart_enable_rx_interrupt(void)
{
    // Read IMSC (Interrupt Mask Set/Clear) register and enable RX interrupt (RXIM = 1)
    writel(readl(uart_base + IMSC) | (1 << 4), uart_base + IMSC);
}

// Function to disable RX interrupt in UART IMSC register
static void uart_disable_rx_interrupt(void)
{
    // Read IMSC and clear RX interrupt (RXIM = 0)
    writel(readl(uart_base + IMSC) & ~(1 << 4), uart_base + IMSC);
}

static void uart_tx(char ch)
{
    while ((readl(uart_base + FR)) & (FR_TX))
        cpu_relax();
    writel(ch, uart_base + DR);
}

static char uart_rx(void)
{
    while (readl(uart_base + FR) & FR_RX)
        cpu_relax();
    return readl(uart_base + DR) & 0xff;
}

static void uart_tx_string(char *str)
{
    while (*str)
        uart_tx(*str);
}

static void led_blink(void)
{
    uart_tx('a');
    char ch = uart_rx();
    gpio_set_value(GPIO_LED, (ch == 'a') ? 1 : 0);
    pr_info("LED is blinking: %s\n", (ch == 'a') ? "ON" : "OFF");
}

static int __init my_init(void)
{
    pr_info("In init\n");

    uart_base = ioremap(UART2_BASE, 0x90);
    if (!uart_base) {
        pr_err("Failed to allocate memory for UART\n");
        return -ENOMEM;
    }

    gpio_base = ioremap(GPIO_BASE, 0x90);
    if (!gpio_base) {
        pr_err("Failed to allocate memory for GPIO\n");
        return -ENOMEM;
    }

    // Configure UART pins (TX, RX)
    int reg = readl(gpio_base + GPFSEL0);
    reg &= ~((7 << 3) | (7 << 0));  // Clear the GPIO14 (TX) and GPIO15 (RX) function
    reg |= (3 << 3) | (3 << 0);     // Set GPIO14 and GPIO15 to ALT0 (UART0)
    writel(reg, gpio_base + GPFSEL0);

    // Disable UART before setting up configuration
    writel(0x0, uart_base + CR);

    // Set baud rate (9600 baud)
    writel(312, uart_base + IBRD);
    writel(32, uart_base + FBRD);

    // Set line control for 8 data bits, no parity, and FIFO enabled
    writel(0x70, uart_base + LCRH);

    // Enable UART, TX, and RX
    writel((1 << 0) | (1 << 8) | (1 << 9), uart_base + CR);

    // Enable RX interrupt in UART IMSC register
    uart_enable_rx_interrupt();

    // Configure GPIO pin for LED output
    int f = readl(gpio_base + GPFSEL2);
    f &= ~(7 << 3);  // Clear GPIO21 (LED)
    f |= (1 << 3);   // Set GPIO21 as output
    writel(f, gpio_base + GPFSEL2);

    // Enable the interrupt in the system (this is the basic interrupt enable process)
    // For the Pi 4, UART2 interrupts are typically routed through the GIC (Generic Interrupt Controller)
    // At this level, we assume that the interrupt controller is handling interrupts as needed.

    pr_info("UART RX Interrupt enabled\n");

    // Test the UART communication
    led_blink();

    return 0;
}

static void __exit my_exit(void)
{
    pr_info("Exiting UART module\n");

    // Disable RX interrupt in UART IMSC register
    uart_disable_rx_interrupt();

    // Disable UART, turn off LED
    gpio_set_value(GPIO_LED, 0);

    // Unmap I/O memory
    iounmap(uart_base);
    iounmap(gpio_base);

    pr_info("UART and GPIO resources freed\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("UART2 with RX Interrupt at register level");
