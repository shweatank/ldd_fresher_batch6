#ifndef RTC_H
#define RTC_H

// DS1307 RTC I2C slave address
#define DS1307_ADDR 0x68

// BSC1 (I2C controller) base address
#define BSC1_BASE (0xFE000000UL + 0x804000)


// I2C Control Register
#define BSC_C      0x00

// I2C Status Register
#define BSC_S      0x04

// Data Length Register
#define BSC_DLEN   0x08

// Slave Address Register
#define BSC_A      0x0C

// FIFO Register
#define BSC_FIFO   0x10

// Clock Divider Register
#define BSC_DIV    0x14


// Clear FIFO bit
#define BSC_CLEAR  (1 << 4)

// Enable I2C controller
#define BSC_I2CEN  (1 << 15)

// Start transfer bit
#define BSC_ST     (1 << 7)

// Read operation bit
#define BSC_READ   (1 << 0)


// Transfer complete flag
#define BSC_DONE   (1 << 1)

// RX FIFO contains data
#define BSC_RXD    (1 << 5)


// Timeout value
#define I2C_TIMEOUT 100000


// Global I2C virtual address pointer
extern void __iomem *i2c;


// Configure GPIO2 and GPIO3 for I2C
void i2c_gpio_init(void);


// Read RTC data from DS1307
int ds1307_read(u8 *buf);


// Convert BCD to decimal
int bcd2bin(u8 val);


// Convert decimal to BCD
u8 bin2bcd(int val);

#endif
