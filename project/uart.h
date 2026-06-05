#ifndef UART_H
#define UART_H

// UART0 base address
#define UART0_BASE (0xFE000000UL + 0x201000)

// UART Data Register
// Used to send and receive data
#define UART_DR     0x00

// UART Flag Register
// Contains UART status flags
#define UART_FR     0x18

// Integer Baud Rate Divisor Register
#define UART_IBRD   0x24

// Fractional Baud Rate Divisor Register
#define UART_FBRD   0x28

// Line Control Register
// Used to configure:
// word length
// parity
// stop bits
#define UART_LCRH   0x2C

// UART Control Register
// Used to enable UART, TX, RX
#define UART_CR     0x30

// TX FIFO FULL flag
// If set, UART cannot accept more data
#define TXFF    (1 << 5)

// UART BUSY flag
// UART still transmitting data
#define BUSY    (1 << 3)

// UART Enable bit
#define UARTEN  (1 << 0)

// Transmit Enable bit
#define TXE     (1 << 8)

// Receive Enable bit
#define RXE     (1 << 9)

// Global UART virtual address pointer
extern void __iomem *uart;

// Configure GPIO14 and GPIO15 for UART
void uart_gpio_init(void);

// Initialize UART peripheral
void uart_init(void);

// Send string through UART
void uart_puts(const char *s);

#endif
