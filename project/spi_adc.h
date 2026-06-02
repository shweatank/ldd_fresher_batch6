#ifndef SPI_ADC_H
#define SPI_ADC_H

// SPI0 controller base address
#define SPI0_BASE (0xFE000000UL + 0x204000)


// SPI Control and Status Register
// Used to:
// enable SPI
// start transfer
// clear FIFO
// check TX/RX status
#define SPI_CS     0x00


// SPI FIFO Register
// Used for:
// transmitting data
// receiving data
#define SPI_FIFO   0x04


// SPI Clock Divider Register
// Controls SPI clock speed
#define SPI_CLK    0x08



// TX FIFO can accept data
#define SPI_TXD    (1 << 18)


// RX FIFO contains received data
#define SPI_RXD    (1 << 17)


// SPI transfer complete flag
#define SPI_DONE   (1 << 16)


// Transfer Active bit
// Enables SPI transfer
#define SPI_TA     (1 << 7)


// Clear TX and RX FIFO
#define SPI_CLEAR  ((1 << 4) | (1 << 5))



// Global SPI virtual address pointer
extern void __iomem *spi;



// Configure GPIO pins for SPI
// GPIO9  -> MISO
// GPIO10 -> MOSI
// GPIO11 -> SCLK
void spi_gpio_init(void);


// Initialize SPI controller
void spi_init(void);


// Transfer one byte through SPI
u8 spi_transfer(u8 val);


// Read MCP3208 ADC value
int mcp3208_read(void);

#endif
