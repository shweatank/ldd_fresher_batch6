#include <linux/module.h>     
// Kernel module support

#include <linux/kernel.h>     
// printk()

#include <linux/io.h>         
// readl(), writel()

#include "gpio.h"             
// GPIO register definitions

#include "spi_adc.h"          
// SPI register definitions


// Global SPI virtual address pointer
void __iomem *spi;


// Configure GPIO pins for SPI
void spi_gpio_init(void)
{
    unsigned int reg;

    // Read GPIO Function Select Register 0
    reg = readl(gpio + GPFSEL0);

    // ============================================
    // GPIO9 -> MISO
    // ============================================

    // Clear GPIO9 function bits
    reg &= ~(7 << 27);

    // Set GPIO9 to ALT0
    reg |=  (4 << 27);

    // Write updated value
    writel(reg, gpio + GPFSEL0);


    // Read GPIO Function Select Register 1
    reg = readl(gpio + GPFSEL1);

    // ============================================
    // GPIO10 -> MOSI
    // ============================================

    // Clear GPIO10 function bits
    reg &= ~(7 << 0);

    // Set GPIO10 to ALT0
    reg |=  (4 << 0);

    // ============================================
    // GPIO11 -> SCLK
    // ============================================

    // Clear GPIO11 function bits
    reg &= ~(7 << 3);

    // Set GPIO11 to ALT0
    reg |=  (4 << 3);

    // Write updated value
    writel(reg, gpio + GPFSEL1);
}


// Initialize SPI controller
void spi_init(void)
{
    // Clear TX and RX FIFO
    writel(SPI_CLEAR,
           spi + SPI_CS);

    // Set SPI clock divider
    // Controls SPI speed
    writel(64,spi + SPI_CLK);
}


// Transfer one byte through SPI
u8 spi_transfer(u8 val)
{
    // Wait until TX FIFO can accept data
    while (!(readl(spi + SPI_CS) & SPI_TXD));

    // Write byte into FIFO
    writel(val,
           spi + SPI_FIFO);

    // Wait until RX FIFO contains data
    while (!(readl(spi + SPI_CS) & SPI_RXD));

    // Return received byte
    return readl(spi + SPI_FIFO);
}


// Read ADC value from MCP3208
int mcp3208_read(void)
{
    u8 r1, r2, r3;

    // Enable SPI transfer
    writel(readl(spi + SPI_CS) | SPI_TA,
           spi + SPI_CS);

    // ============================================
    // MCP3208 COMMAND SEQUENCE
    // ============================================

    // Send start bit + single-ended mode
    r1 = spi_transfer(0x06);

    // Send channel selection
    // Channel 0 selected
    r2 = spi_transfer(0x00);

    // Dummy byte to receive remaining data
    r3 = spi_transfer(0x00);

    // Wait until SPI transfer completes
    while (!(readl(spi + SPI_CS) & SPI_DONE));

    // Disable SPI transfer
    writel(readl(spi + SPI_CS) & ~SPI_TA,
           spi + SPI_CS);

    // Combine 12-bit ADC result
    return ((r2 & 0x0F) << 8) | r3;
}
