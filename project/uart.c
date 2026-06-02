#include <linux/module.h>     
// Kernel module support

#include <linux/kernel.h>     
// printk()

#include <linux/io.h>         
// readl(), writel()

#include "gpio.h"             
// GPIO register definitions

#include "uart.h"             
// UART register definitions


// Global UART virtual address pointer
void __iomem *uart;


// Configure GPIO14 and GPIO15 for UART
void uart_gpio_init(void)
{
    unsigned int reg;

    // Read GPIO Function Select Register 1
    reg = readl(gpio + GPFSEL1);

    // Clear GPIO14 function bits
    reg &= ~(7 << 12);

    // Set GPIO14 to ALT0 (UART TX)
    reg |=  (4 << 12);

    // Clear GPIO15 function bits
    reg &= ~(7 << 15);

    // Set GPIO15 to ALT0 (UART RX)
    reg |=  (4 << 15);

    // Write updated register value
    writel(reg, gpio + GPFSEL1);
}


// Initialize UART peripheral
void uart_init(void)
{
    // Disable UART before configuration
    writel(0x0, uart + UART_CR);

    // Set integer baud rate divisor
    writel(26, uart + UART_IBRD);

    // Set fractional baud rate divisor
    writel(3, uart + UART_FBRD);

    // Configure:
    // 8-bit data
    // No parity
    // 1 stop bit
    writel((3 << 5), uart + UART_LCRH);

    // Enable:
    // UART
    // Transmitter
    // Receiver
    writel(UARTEN | TXE | RXE,
           uart + UART_CR);
}


// Send string through UART
void uart_puts(const char *s)
{
    // Loop until NULL character
    while (*s)
    {
        // Wait if TX FIFO is full
        while (readl(uart + UART_FR) & TXFF);

        // Send one character
        writel(*s++, uart + UART_DR);
    }

    // Wait until UART transmission completes
    while (readl(uart + UART_FR) & BUSY);
}
