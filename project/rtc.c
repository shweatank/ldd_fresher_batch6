#include <linux/module.h>     
// Kernel module support

#include <linux/kernel.h>     
// printk()

#include <linux/io.h>         
// readl(), writel()

#include <linux/delay.h>      
// delay functions

#include "gpio.h"             
// GPIO register definitions

#include "rtc.h"              
// RTC/I2C definitions


// Global I2C virtual address pointer
void __iomem *i2c;


// Configure GPIO2 and GPIO3 for I2C
void i2c_gpio_init(void)
{
	unsigned int reg;

	// Read GPIO Function Select Register 0
	reg = readl(gpio + GPFSEL0);

	// Clear GPIO2 function bits
	reg &= ~(7 << 6);

	// Set GPIO2 to ALT0 (SDA)
	reg |=  (4 << 6);

	// Clear GPIO3 function bits
	reg &= ~(7 << 9);

	// Set GPIO3 to ALT0 (SCL)
	reg |=  (4 << 9);

	// Write updated register value
	writel(reg, gpio + GPFSEL0);
}


// Convert BCD to decimal
int bcd2bin(u8 val)
{
	// Convert:
	// upper nibble -> tens digit
	// lower nibble -> ones digit

	return (val & 0x0F) +
		((val >> 4) * 10);
}


// Convert decimal to BCD
u8 bin2bcd(int val)
{
	// Convert decimal to BCD format

	return ((val / 10) << 4) |
		(val % 10);
}


// Read RTC values from DS1307
int ds1307_read(u8 *buf)
{
	int i = 0;
	int timeout;

	// Set DS1307 slave address
	writel(DS1307_ADDR,
			i2c + BSC_A);

	// Transfer length = 1 byte
	writel(1,
			i2c + BSC_DLEN);

	// Select RTC register 0x00
	// Start reading from seconds register
	writel(0x00,
			i2c + BSC_FIFO);

	// Clear DONE flag
	writel(BSC_DONE,
			i2c + BSC_S);

	// Start I2C write transfer
	writel(BSC_I2CEN | BSC_ST,
			i2c + BSC_C);

	// Wait until transfer completes
	timeout = I2C_TIMEOUT;

	while (!(readl(i2c + BSC_S) & BSC_DONE))
	{
		// Timeout protection
		if (--timeout == 0)
			return -1;

		cpu_relax();
	}


	// Read 7 RTC bytes
	writel(7,
			i2c + BSC_DLEN);

	// Clear DONE flag
	writel(BSC_DONE,
			i2c + BSC_S);

	// Start I2C read transfer
	writel(BSC_I2CEN |
			BSC_ST |
			BSC_READ,
			i2c + BSC_C);

	timeout = I2C_TIMEOUT;

	// Read all 7 bytes
	while (i < 7)
	{
		// Check RX FIFO has data
		if (readl(i2c + BSC_S) & BSC_RXD)
		{
			// Read byte from FIFO
			buf[i++] =
				readl(i2c + BSC_FIFO);

			// Reset timeout
			timeout = I2C_TIMEOUT;
		}
		else
		{
			// Timeout protection
			if (--timeout == 0)
				return -1;

			cpu_relax();
		}
	}

	return 0;
}
