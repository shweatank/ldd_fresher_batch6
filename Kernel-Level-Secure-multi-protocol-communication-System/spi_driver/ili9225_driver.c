/*
 * Simple SPI driver for ILI9225-based 176x220 TFT LCD panels.
 * Provides a character device interface for writing text to the display.
 *
 * This driver is designed for educational purposes and demonstrates basic
 * SPI communication, GPIO control, and character rendering on the ILI9225.
 * Note: This is a simplified driver and may not include all features or optimizations for production use.
 */
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

/* Driver names used in kernel/device registration */
#define DRIVER_NAME "ili9225"
#define DEVICE_NAME "ili9225_char"
#define CLASS_NAME  "ili"

/* Display resolution for ILI9225 */
#define SCREEN_WIDTH 176
#define SCREEN_HEIGHT 220

/*
 * 8x8 bitmap font table for printable ASCII characters.
 * Index mapping: font8x8_basic[c - 32]
 */
static const unsigned char font8x8_basic[96][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00},
    {0x6C,0x6C,0x48,0x00,0x00,0x00,0x00,0x00}, {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00},
    {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00}, {0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00},
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00}, {0x30,0x30,0x60,0x00,0x00,0x00,0x00,0x00},
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00},
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00},
    {0x7C,0xC6,0xCE,0xDE,0xF6,0xE6,0x7C,0x00}, {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
    {0x7C,0xC6,0x0E,0x1C,0x70,0xC6,0xFE,0x00}, {0x7C,0xC6,0x06,0x3C,0x06,0xC6,0x7C,0x00},
    {0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x1E,0x00}, {0xFE,0xC0,0xFC,0x06,0x06,0xC6,0x7C,0x00},
    {0x3C,0x60,0xC0,0xFC,0xC6,0xC6,0x7C,0x00}, {0xFE,0xC6,0x0C,0x18,0x30,0x30,0x30,0x00},
    {0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0x7C,0x00}, {0x7C,0xC6,0xC6,0x7E,0x06,0x0C,0x78,0x00},
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00}, {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30},
    {0x0E,0x1C,0x38,0x70,0x38,0x1C,0x0E,0x00}, {0x00,0x00,0x7E,0x00,0x00,0x7E,0x00,0x00},
    {0x70,0x38,0x1C,0x0E,0x1C,0x38,0x70,0x00}, {0x7C,0xC6,0x0E,0x1C,0x18,0x00,0x18,0x00},
    {0x7C,0xC6,0xDE,0xDE,0xDE,0xC0,0x7C,0x00}, {0x38,0x6C,0xC6,0xFE,0xC6,0xC6,0xC6,0x00},
    {0xFC,0x66,0x66,0x7C,0x66,0x66,0xFC,0x00}, {0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00},
    {0xF8,0x6C,0x66,0x66,0x66,0x6C,0xF8,0x00}, {0xFE,0x62,0x68,0x78,0x68,0x62,0xFE,0x00},
    {0xFE,0x62,0x68,0x78,0x68,0x60,0xF0,0x00}, {0x3C,0x66,0xC0,0xC0,0xCE,0x66,0x3E,0x00},
    {0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00}, {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},
    {0x1E,0x0C,0x0C,0x0C,0xCC,0xCC,0x78,0x00}, {0xE6,0x66,0x6C,0x78,0x6C,0x66,0xE6,0x00},
    {0xF0,0x60,0x60,0x60,0x62,0x66,0xFE,0x00}, {0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00},
    {0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0x00}, {0x38,0x6C,0xC6,0xC6,0xC6,0x6C,0x38,0x00},
    {0xFC,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00}, {0x38,0x6C,0xC6,0xC6,0xDA,0x6C,0x3A,0x00},
    {0xFC,0x66,0x66,0x7C,0x6C,0x66,0xE6,0x00}, {0x7C,0xC6,0x60,0x38,0x0C,0xC6,0x7C,0x00},
    {0x7E,0x7E,0x5A,0x18,0x18,0x18,0x3C,0x00}, {0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00},
    {0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0x00}, {0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00},
    {0xC6,0x6C,0x38,0x10,0x38,0x6C,0xC6,0x00}, {0x66,0x66,0x66,0x3C,0x18,0x18,0x3C,0x00},
    {0xFE,0xC6,0x8C,0x18,0x32,0x66,0xFE,0x00}, {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00},
    {0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00}, {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00},
    {0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00}, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF},
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x7C,0x06,0x7E,0xC6,0x7E,0x00},
    {0xE0,0x60,0x7C,0x66,0x66,0x66,0xDC,0x00}, {0x00,0x00,0x7C,0xC0,0xC0,0xC0,0x7C,0x00},
    {0x1C,0x0C,0x7C,0xCC,0xCC,0xCC,0x76,0x00}, {0x00,0x00,0x7C,0xC6,0xFE,0xC0,0x7C,0x00},
    {0x1C,0x36,0x30,0x78,0x30,0x30,0x78,0x00}, {0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0xF8},
    {0xE0,0x60,0x6C,0x76,0x66,0x66,0xE6,0x00}, {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00},
    {0x0C,0x00,0x1C,0x0C,0x0C,0xCC,0xCC,0x78}, {0xE0,0x60,0x66,0x6C,0x78,0x6C,0xE6,0x00},
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, {0x00,0x00,0xEC,0xFE,0xD6,0xC6,0xC6,0x00},
    {0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x00}, {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0x00},
    {0x00,0x00,0xDC,0x66,0x66,0x7C,0x60,0xF0}, {0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0x1E},
    {0x00,0x00,0xDC,0x76,0x66,0x60,0xF0,0x00}, {0x00,0x00,0x7E,0xC0,0x7C,0x06,0xFC,0x00},
    {0x30,0x30,0xFC,0x30,0x30,0x36,0x1C,0x00}, {0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x76,0x00},
    {0x00,0x00,0xC6,0xC6,0x6C,0x38,0x10,0x00}, {0x00,0x00,0xC6,0xD6,0xFE,0xEE,0xC6,0x00},
    {0x00,0x00,0xC6,0x6C,0x38,0x6C,0xC6,0x00}, {0x00,0x00,0xC6,0xC6,0xC6,0x7E,0x06,0xFC},
    {0x00,0x00,0xFE,0x8C,0x18,0x32,0xFE,0x00}, {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00},
    {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00}, {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00},
    {0x38,0x44,0x44,0x38,0x00,0x00,0x00,0x00},
};

/* Per-device context for SPI display */
struct ili9225 {
    struct spi_device *spi;      /* SPI bus handle */
    struct gpio_desc *dc;        /* Data/Command pin */
    struct gpio_desc *reset;     /* Reset pin */
};

/* Global references (single display instance) */
static struct ili9225 *g_lcd;// Global pointer to the ili9225 device context, used for drawing operations in file operations
static dev_t dev_num;// Device number for character device registration
static struct class *ili_class;// Device class for character device
static struct cdev ili_cdev;// Character device structure

/* Cursor position for text drawing */
static int cursor_x;
static int cursor_y;

/* Send one 16-bit value over SPI (MSB first) */
static int ili9225_write16(struct ili9225 *lcd, u16 value)
{
    u8 buf[2];// Buffer to hold the two bytes of the 16-bit value to be sent over SPI. The value is split into two bytes, with the most significant byte (MSB) placed in buf[0] and the least significant byte (LSB) placed in buf[1], following the MSB-first convention required by the ILI9225 controller.
    buf[0] = value >> 8;// Place the most significant byte (MSB) of the 16-bit value into buf[0] by right-shifting the value by 8 bits. This effectively moves the upper 8 bits of the value into the lower 8 bits of buf[0].
    buf[1] = value & 0xFF;// Place the least significant byte (LSB) of the 16-bit value into buf[1] by performing a bitwise AND operation with 0xFF. This masks out the upper 8 bits of the value, leaving only the lower 8 bits, which are stored in buf[1].
    return spi_write(lcd->spi, buf, 2);// Send the two-byte buffer over the SPI bus using the spi_write function, which takes the SPI device handle (lcd->spi), the buffer containing the data to be sent (buf), and the length of the data in bytes (2). The function returns 0 on success or a negative error code on failure.
}

/* Write one register-value pair to LCD controller */
static void ili9225_write_reg(struct ili9225 *lcd, u16 reg, u16 data)
{
    gpiod_set_value(lcd->dc, 0);         /* command mode */
    ili9225_write16(lcd, reg);// Write the register address to the LCD controller by sending the 16-bit reg value over SPI. Before sending the register address, we set the Data/Command (DC) pin to 0 to indicate that we are sending a command (register address) rather than data.
    gpiod_set_value(lcd->dc, 1);         /* data mode */
    ili9225_write16(lcd, data);// Write the register data to the LCD controller by sending the 16-bit data value over SPI. After sending the register address, we set the Data/Command (DC) pin to 1 to indicate that we are now sending data associated with the previously sent register address.
}

/* Hardware reset pulse for ILI9225 */
static void ili9225_reset(struct ili9225 *lcd)
{
    gpiod_set_value(lcd->reset, 1);// Set the reset pin to high (inactive state) before starting the reset sequence. This ensures that the LCD is not held in reset at the beginning of the sequence.
    msleep(5);// Wait for 5 milliseconds to ensure that the LCD has stabilized before initiating the reset pulse. This delay allows any previous operations to complete and ensures that the LCD is ready for the reset sequence.
    gpiod_set_value(lcd->reset, 0);// Set the reset pin to low (active state) to initiate the hardware reset pulse. This will reset the LCD controller and prepare it for initialization.
    msleep(20);// Wait for 20 milliseconds while the reset pulse is active. This duration ensures that the LCD controller has enough time to recognize the reset signal and perform the necessary internal reset operations.
    gpiod_set_value(lcd->reset, 1);// Set the reset pin back to high (inactive state) to complete the reset sequence. This allows the LCD controller to exit the reset state and begin normal operation.
    msleep(50);//   Wait for 50 milliseconds after releasing the reset pin to ensure that the LCD controller has fully initialized and is ready for further commands. This delay allows the internal circuits of the LCD to stabilize before we start sending initialization commands.
}

/* Initialization register sequence for LCD */
static void ili9225_init(struct ili9225 *lcd)
{
    ili9225_reset(lcd);
    ili9225_write_reg(lcd, 0x01, 0x011C);// Write the value 0x011C to register 0x01 of the ILI9225 LCD controller. This register is typically used for power control settings, and the specific value configures the power control parameters for the LCD. The exact effect of this value depends on the ILI9225 datasheet, but it generally sets up the internal power circuits of the LCD for proper operation.
    ili9225_write_reg(lcd, 0x02, 0x0100);// Write the value 0x0100 to register 0x02 of the ILI9225 LCD controller. This register is often used for power control settings as well, and the value configures additional power parameters for the LCD. The specific effect of this value can be found in the ILI9225 datasheet, but it typically helps to further stabilize the power supply to the LCD.
    ili9225_write_reg(lcd, 0x03, 0x1030);// Write the value 0x1030 to register 0x03 of the ILI9225 LCD controller. This register is commonly used for power control settings, and the value configures the power supply parameters for the LCD. The specific effect of this value can be referenced in the ILI9225 datasheet, but it generally helps to ensure that the LCD receives the correct voltage levels for operation.
    ili9225_write_reg(lcd, 0x08, 0x0808);// Write the value 0x0808 to register 0x08 of the ILI9225 LCD controller. This register is typically used for display control settings, and the value configures parameters such as the display mode, color format, or other display-related settings. The exact effect of this value can be found in the ILI9225 datasheet, but it generally helps to set up the display parameters for proper rendering.
    ili9225_write_reg(lcd, 0x0F, 0x0B01);// Write the value 0x0B01 to register 0x0F of the ILI9225 LCD controller. This register is often used for display control settings, and the value configures additional display parameters such as the scanning direction, color order, or other display-related settings. The specific effect of this value can be referenced in the ILI9225 datasheet, but it generally helps to further configure the display parameters for optimal performance.
    ili9225_write_reg(lcd, 0x10, 0x0A00);// Write the value 0x0A00 to register 0x10 of the ILI9225 LCD controller. This register is commonly used for display control settings, and the value configures parameters such as the entry mode, color format, or other display-related settings. The exact effect of this value can be found in the ILI9225 datasheet, but it generally helps to set up the display parameters for proper rendering and color handling.
    ili9225_write_reg(lcd, 0x11, 0x1038);// Write the value 0x1038 to register 0x11 of the ILI9225 LCD controller. This register is commonly used for display control settings, and the value configures parameters such as the entry mode, color format, or other display-related settings. The exact effect of this value can be found in the ILI9225 datasheet, but it generally helps to set up the display parameters for proper rendering and color handling.
    msleep(50);// Wait for 50 milliseconds after sending the initial display control commands to ensure that the LCD controller has enough time to process these settings and stabilize before we continue with further initialization commands. This delay allows the internal circuits of the LCD to adjust to the new settings before we proceed with additional configuration.
    ili9225_write_reg(lcd, 0x12, 0x1121);// Write the value 0x1121 to register 0x12 of the ILI9225 LCD controller. This register is typically used for display control settings, and the value configures parameters such as the entry mode, color format, or other display-related settings. The specific effect of this value can be found in the ILI9225 datasheet, but it generally helps to further configure the display parameters for optimal performance and proper rendering.
    ili9225_write_reg(lcd, 0x13, 0x0063);// Write the value 0x0063 to register 0x13 of the ILI9225 LCD controller. This register is often used for display control settings, and the value configures additional display parameters such as the scanning direction, color order, or other display-related settings. The specific effect of this value can be referenced in the ILI9225 datasheet, but it generally helps to further configure the display parameters for optimal performance.
    ili9225_write_reg(lcd, 0x14, 0x5A00);// Write the value 0x5A00 to register 0x14 of the ILI9225 LCD controller. This register is commonly used for display control settings, and the value configures parameters such as the entry mode, color format, or other display-related settings. The exact effect of this value can be found in the ILI9225 datasheet, but it generally helps to set up the display parameters for proper rendering and color handling.
    msleep(50);// Wait for 50 milliseconds after sending the display control commands to ensure that the LCD controller has enough time to process these settings and stabilize before we continue with further initialization commands. This delay allows the internal circuits of the LCD to adjust to the new settings before we proceed with additional configuration.
    ili9225_write_reg(lcd, 0x07, 0x1017);// Write the value 0x1017 to register 0x07 of the ILI9225 LCD controller. This register is typically used for display control settings, and the value configures parameters such as the display mode, color format, or other display-related settings. The specific effect of this value can be found in the ILI9225 datasheet, but it generally helps to finalize the display parameters and prepare the LCD for normal operation.
}

/* Fill complete screen with a single RGB565 color */
static void ili9225_fill(struct ili9225 *lcd, u16 color)
{
    int x, y;

    /* Set drawing window: full panel */
    ili9225_write_reg(lcd, 0x36, 175);// Write the value 175 to register 0x36 of the ILI9225 LCD controller. This register is used to set the horizontal start and end positions of the drawing window.
    ili9225_write_reg(lcd, 0x37, 0);// Write the value 0 to register 0x37 of the ILI9225 LCD controller. This register is used to set the vertical start and end positions of the drawing window.
    ili9225_write_reg(lcd, 0x38, 219);// Write the value 219 to register 0x38 of the ILI9225 LCD controller. This register is used to set the vertical start and end positions of the drawing window.
    ili9225_write_reg(lcd, 0x39, 0);// Write the value 0 to register 0x39 of the ILI9225 LCD controller. This register is used to set the vertical start and end positions of the drawing window.

    /* Set GRAM start address */
    ili9225_write_reg(lcd, 0x20, 0);// Write the value 0 to register 0x20 of the ILI9225 LCD controller. This register is used to set the horizontal GRAM (Graphics RAM) start address for subsequent memory write operations.
    ili9225_write_reg(lcd, 0x21, 0);// Write the value 0 to register 0x21 of the ILI9225 LCD controller. This register is used to set the vertical GRAM (Graphics RAM) start address for subsequent memory write operations.

    /* Start memory write */
    gpiod_set_value(lcd->dc, 0);// Set the Data/Command (DC) pin to 0 to indicate that we are sending a command (in this case, the memory write command) rather than data. This prepares the LCD controller to receive the command for starting a memory write operation.
    ili9225_write16(lcd, 0x22);// Write the value 0x22 to the LCD controller to indicate that we want to start writing pixel data to the GRAM (Graphics RAM). This command tells the LCD controller that subsequent data sent over SPI will be pixel data that should be written to the specified GRAM address.
    gpiod_set_value(lcd->dc, 1);//  Set the Data/Command (DC) pin to 1 to indicate that we are now sending data (pixel color values) rather than commands. This prepares the LCD controller to receive pixel data for the memory write operation.

    /* Push one color for every pixel */
    for (y = 0; y < SCREEN_HEIGHT; y++)// Loop through each row of the screen, iterating from 0 to SCREEN_HEIGHT - 1. This outer loop controls the vertical position of the pixels being filled on the LCD.
        for (x = 0; x < SCREEN_WIDTH; x++)// Loop through each column of the screen, iterating from 0 to SCREEN_WIDTH - 1. This inner loop controls the horizontal position of the pixels being filled on the LCD.
            ili9225_write16(lcd, color);// Write the specified color value to the LCD controller for each pixel on the screen. Since we have already sent the memory write command (0x22) and set the DC pin to indicate data mode, each call to ili9225_write16 will send a pixel color value that fills the entire screen with the specified color.
}

/* Draw one pixel at (x, y) */
static void drawPixel(int x, int y, u16 color)
{
    struct ili9225 *lcd = g_lcd;// Get the global LCD device context to access the SPI and GPIO interfaces for drawing operations. This allows us to send commands and data to the LCD controller to set the pixel color at the specified coordinates.

    gpiod_set_value(lcd->dc, 0); ili9225_write16(lcd, 0x20);// Set the horizontal GRAM (Graphics RAM) address to x by writing the value 0x20 followed by the x coordinate. This tells the LCD controller where to position the pixel horizontally in the GRAM for the subsequent pixel data write.
    gpiod_set_value(lcd->dc, 1); ili9225_write16(lcd, x);// Write the x coordinate to the LCD controller to set the horizontal GRAM address for the pixel we want to draw. This is done after setting the DC pin to indicate that we are sending data (the coordinate value) rather than a command.
    gpiod_set_value(lcd->dc, 0); ili9225_write16(lcd, 0x21);// Set the vertical GRAM (Graphics RAM) address to y by writing the value 0x21 followed by the y coordinate. This tells the LCD controller where to position the pixel vertically in the GRAM for the subsequent pixel data write.
    gpiod_set_value(lcd->dc, 1); ili9225_write16(lcd, y);// Write the y coordinate to the LCD controller to set the vertical GRAM address for the pixel we want to draw. This is done after setting the DC pin to indicate that we are sending data (the coordinate value) rather than a command.
    gpiod_set_value(lcd->dc, 0); ili9225_write16(lcd, 0x22);// Send the memory write command (0x22) to the LCD controller to indicate that we want to write pixel data to the GRAM at the specified coordinates. This prepares the LCD controller to receive the pixel color data for the pixel we are about to draw.
    gpiod_set_value(lcd->dc, 1); ili9225_write16(lcd, color);// Write the specified color value to the LCD controller for the pixel at coordinates (x, y). Since we have already set the GRAM address and sent the memory write command, this call to ili9225_write16 will set the color of the pixel at the specified location on the LCD.
}

/* Draw one scaled character (2x scale from 8x8 bitmap) */
static void drawChar(int x, int y, char c, u16 color)
{
    int row, col;// Loop variables for iterating through the rows and columns of the character bitmap. The character is represented as an 8x8 pixel bitmap, and we will scale it to 16x16 pixels by drawing each pixel as a 2x2 block on the LCD.
    const u8 *bitmap;// Pointer to the bitmap data for the character to be drawn. The bitmap is an array of 8 bytes, where each byte represents one row of the 8x8 character. Each bit in the byte corresponds to a pixel in that row, with a value of 1 indicating that the pixel should be drawn (colored) and a value of 0 indicating that the pixel should be left blank.

    if (c < 32 || c > 127)// Check if the character code is outside the range of printable ASCII characters (32 to 127). If it is, we simply return without drawing anything, as we do not have bitmap data for non-printable characters.
        return;

    bitmap = font8x8_basic[c - 32];// Get the bitmap data for the character by indexing into the font8x8_basic array using the character code minus 32 (to account for the offset of printable characters in the ASCII table). This gives us a pointer to the 8-byte bitmap that represents the character we want to draw.

    for (row = 0; row < 8; row++) {// Loop through each row of the character bitmap, iterating from 0 to 7. This outer loop controls the vertical position of the pixels being drawn for the character.
        for (col = 0; col < 8; col++) {// Loop through each column of the character bitmap, iterating from 0 to 7. This inner loop controls the horizontal position of the pixels being drawn for the character.
            if (bitmap[row] & (1 << (7 - col))) {// Check if the bit corresponding to the current column in the current row of the bitmap is set (i.e., if it is 1). We do this by performing a bitwise AND operation between the byte for the current row (bitmap[row]) and a mask that has a single bit set at the position corresponding to the current column (1 << (7 - col)). If the result is non-zero, it means that the pixel at this position should be drawn with the specified color.
                drawPixel(x + col * 2,     y + row * 2,     color);// Draw the pixel at the top-left corner of the 2x2 block for the current character pixel. The x coordinate is calculated by adding the column index multiplied by 2 to the starting x coordinate, and the y coordinate is calculated by adding the row index multiplied by 2 to the starting y coordinate. This effectively scales the character pixel to a 2x2 block on the LCD.
                drawPixel(x + col * 2 + 1, y + row * 2,     color);// Draw the pixel at the top-right corner of the 2x2 block for the current character pixel. The x coordinate is calculated by adding 1 to the previous x coordinate, while the y coordinate remains the same as it is still in the same row of the block.
                drawPixel(x + col * 2,     y + row * 2 + 1, color);// Draw the pixel at the bottom-left corner of the 2x2 block for the current character pixel. The x coordinate remains the same as the first pixel in the block, while the y coordinate is calculated by adding 1 to the previous y coordinate, placing it in the next row of the block.
                drawPixel(x + col * 2 + 1, y + row * 2 + 1, color);// Draw the pixel at the bottom-right corner of the 2x2 block for the current character pixel. The x coordinate is calculated by adding 1 to the previous x coordinate, and the y coordinate is calculated by adding 1 to the previous y coordinate, placing it in the next row of the block. This completes the 2x2 block for the current character pixel, effectively scaling it up on the LCD.
            }
        }
    }
}

/* Draw a full string with newline and wrapping support */
static void drawString(const char *str)
{
    int i, start_x = cursor_x;// Store the initial x coordinate of the cursor before drawing the string. This allows us to reset the x coordinate back to this starting position when we encounter a newline character or when we need to wrap text to the next line.

    for (i = 0; str[i]; i++) {// Loop through each character in the input string until we reach the null terminator. The variable i is used as an index to access each character in the string.
        if (str[i] == '\n') {// Check if the current character is a newline character ('\n'). If it is, we need to move the cursor to the beginning of the next line before drawing the next characters.
            cursor_x = start_x;// Reset the x coordinate of the cursor back to the starting position (the leftmost position) for the new line. This ensures that the next characters will be drawn starting from the left edge of the screen.
            cursor_y += 16;// Move the y coordinate of the cursor down by 16 pixels to position it on the next line. Since each character is drawn as a 16x16 block, we need to move down by 16 pixels to avoid overlapping with the previous line of text.
            continue;// Skip the rest of the loop body and move on to the next character in the string, as we have already handled the newline character by moving the cursor to the next line.
        }

        drawChar(cursor_x, cursor_y, str[i], 0x0000);// Draw the current character at the current cursor position (cursor_x, cursor_y) with the color 0x0000 (black). The drawChar function will handle the actual drawing of the character on the LCD based on its bitmap representation.
        cursor_x += 16;// Move the x coordinate of the cursor to the right by 16 pixels to position it for the next character. Since each character is drawn as a 16x16 block, we need to move the cursor by 16 pixels to avoid overlapping with the previous character.

        if (cursor_x > SCREEN_WIDTH - 16) {// Check if the x coordinate of the cursor has exceeded the width of the screen minus 16 pixels (the width of one character block). If it has, we need to wrap the text to the next line to ensure that characters do not go off the edge of the screen.
            cursor_x = start_x;// Reset the x coordinate of the cursor back to the starting position (the leftmost position) for the new line. This ensures that the next characters will be drawn starting from the left edge of the screen when we wrap to the next line.
            cursor_y += 16;// Move the y coordinate of the cursor down by 16 pixels to position it on the next line. This allows us to continue drawing characters on the next line without overlapping with the previous line of text.
        }
    }
}

/*
 * Character device write:
 * - receives text from user space
 * - clears display
 * - renders received text
 */
static ssize_t ili_write(struct file *file, const char __user *buf, size_t len, loff_t *off)//
{
    char kbuf[256];// Kernel buffer to hold the data copied from user space. We define a fixed-size buffer of 256 bytes to store the text that will be written to the LCD. The size of this buffer limits the maximum length of the text that can be written in one operation.

    if (len > sizeof(kbuf) - 1)// Check if the length of the data to be written exceeds the size of the kernel buffer minus one byte (to leave space for the null terminator). If it does, we limit the length to the maximum size that can be safely copied into the buffer.
        len = sizeof(kbuf) - 1;// Adjust the length to ensure that we do not overflow the kernel buffer when copying data from user space. This prevents potential buffer overflow vulnerabilities and ensures that we only copy as much data as can fit in the buffer.

    if (copy_from_user(kbuf, buf, len))// Copy data from user space to the kernel buffer. The copy_from_user function returns a non-zero value if the copy operation fails (e.g., if the user space pointer is invalid). If the copy fails, we return -EFAULT to indicate a bad memory address error.
        return -EFAULT;// If the copy operation is successful, we proceed to null-terminate the kernel buffer to ensure that it can be safely used as a string for rendering on the LCD. This is important because the drawing functions expect a null-terminated string to determine where the text ends.

    kbuf[len] = '\0';// Null-terminate the kernel buffer after copying data from user space. This ensures that the buffer can be safely used as a string for rendering on the LCD, as the drawing functions rely on the null terminator to determine the end of the string.

    ili9225_fill(g_lcd, 0xFFFF);   /* white background */
    cursor_x = 0;// Reset the x coordinate of the cursor to 0 to start drawing from the left edge of the screen. This ensures that the text will be drawn starting from the leftmost position after we clear the display.
    cursor_y = 20;// Reset the y coordinate of the cursor to 20 to start drawing from the top of the screen. This ensures that the text will be drawn starting from the topmost position after we clear the display.
    drawString(kbuf);// Draw the string contained in the kernel buffer on the LCD using the drawString function. This function will handle the rendering of the text on the LCD based on the character bitmaps and the current cursor position.

    return len;// Return the number of bytes that were written, which is the length of the input data. This indicates to the user space application how many bytes were successfully processed and rendered on the LCD.
}

/* File operations for /dev/ili9225_char */
static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = ili_write,
};

/*
 * Probe is called when matching SPI device appears.
 * Here we:
 * 1) allocate context
 * 2) get GPIOs from DT
 * 3) configure SPI
 * 4) init panel
 * 5) create char device node
 */
static int ili9225_probe(struct spi_device *spi)// The probe function is called by the SPI core when a matching SPI device is detected (based on the compatible string in the device tree). This function is responsible for initializing the device, setting up the necessary resources, and preparing it for operation. The steps include allocating a context structure for the device, retrieving GPIOs from the device tree, configuring the SPI interface, initializing the LCD panel, and creating a character device node for user space interaction.
{
    struct ili9225 *lcd;// Pointer to the device context structure for the ILI9225 LCD. This structure will hold references to the SPI device, GPIOs, and any other relevant information needed for operating the LCD. We will allocate memory for this structure and initialize it with the necessary resources during the probe function.
    int ret;// Variable to hold return values from various operations during the probe function. This allows us to check for errors at each step and handle them appropriately, ensuring that we can clean up resources if something goes wrong during initialization.

    lcd = devm_kzalloc(&spi->dev, sizeof(*lcd), GFP_KERNEL);// Allocate memory for the device context structure using devm_kzalloc, which automatically handles cleanup when the device is removed. We pass the device pointer (spi->dev), the size of the structure, and the GFP_KERNEL flag to indicate that we want to allocate normal kernel memory. The allocated memory is zero-initialized, ensuring that all fields in the structure start with a default value of 0 or NULL.
    if (!lcd)// Check if the memory allocation for the device context structure failed. If devm_kzalloc returns NULL, it indicates that the allocation was unsuccessful, and we return -ENOMEM to indicate an out-of-memory error.
        return -ENOMEM;// If the allocation is successful, we proceed to initialize the device context structure with the necessary resources and configurations for the ILI9225 LCD.

    lcd->spi = spi;// Store the SPI device handle in the device context structure. This allows us to access the SPI interface for sending commands and data to the LCD controller throughout the driver's operations.
    spi_set_drvdata(spi, lcd);// Associate the device context structure with the SPI device using spi_set_drvdata. This allows us to retrieve the context later using spi_get_drvdata when we need to access the device context in other parts of the driver (e.g., in file operations or during removal).

    lcd->dc = devm_gpiod_get(&spi->dev, "dc", GPIOD_OUT_LOW);// Retrieve the Data/Command (DC) GPIO from the device tree using devm_gpiod_get. We specify the device pointer (spi->dev), the name of the GPIO ("dc"), and the initial output value (GPIOD_OUT_LOW) to set the DC pin to a known state. The devm_ function will automatically handle cleanup of this GPIO when the device is removed.
    if (IS_ERR(lcd->dc))// Check if the retrieval of the DC GPIO failed. If devm_gpiod_get returns an error pointer, it indicates that the GPIO could not be obtained (e.g., if it is not defined in the device tree or if there is a hardware issue). In this case, we return the error code using PTR_ERR to indicate the specific error that occurred during GPIO retrieval.
        return PTR_ERR(lcd->dc);

    lcd->reset = devm_gpiod_get(&spi->dev, "reset", GPIOD_OUT_HIGH);// Retrieve the Reset GPIO from the device tree using devm_gpiod_get. We specify the device pointer (spi->dev), the name of the GPIO ("reset"), and the initial output value (GPIOD_OUT_HIGH) to set the reset pin to a known state. The devm_ function will automatically handle cleanup of this GPIO when the device is removed.
    if (IS_ERR(lcd->reset))// Check if the retrieval of the Reset GPIO failed. If devm_gpiod_get returns an error pointer, it indicates that the GPIO could not be obtained (e.g., if it is not defined in the device tree or if there is a hardware issue). In this case, we return the error code using PTR_ERR to indicate the specific error that occurred during GPIO retrieval.
        return PTR_ERR(lcd->reset);// If both GPIOs are successfully retrieved, we proceed to configure the SPI interface for communication with the ILI9225 LCD controller.

    spi->mode = SPI_MODE_0;// Set the SPI mode to SPI_MODE_0, which means that the clock polarity (CPOL) is 0 and the clock phase (CPHA) is 0. This configuration is required by the ILI9225 LCD controller for proper communication over the SPI bus.
    spi->max_speed_hz = 10000000;// Set the maximum SPI clock speed to 10 MHz (10000000 Hz). This value is chosen based on the capabilities of the ILI9225 LCD controller and the requirements for reliable communication. The actual speed may be limited by the hardware, so it's important to choose a value that is supported by both the controller and the SPI master.
    spi->bits_per_word = 8;// Set the number of bits per word for SPI communication to 8. This means that each SPI transfer will consist of 8 bits (1 byte) of data. The ILI9225 LCD controller expects data to be sent in 8-bit chunks, so this configuration ensures that the SPI transfers are formatted correctly for the controller.

    ret = spi_setup(spi);// Call spi_setup to apply the SPI configuration settings we have made (mode, max speed, bits per word). This function will configure the SPI master controller with the specified settings and prepare it for communication with the ILI9225 LCD controller. If spi_setup returns a non-zero value, it indicates that there was an error during the setup process, and we return that error code to indicate the failure.
    if (ret)// Check if the SPI setup was successful. If spi_setup returns a non-zero value, it indicates that there was an error during the setup process (e.g., invalid configuration, hardware issue). In this case, we return the error code to indicate the failure and prevent further initialization of the device.
        return ret;

    ili9225_init(lcd);// Call the ili9225_init function to perform the initialization sequence for the ILI9225 LCD controller. This function will send a series of commands and data to the LCD to configure it for normal operation, including setting up power control, display control, and other necessary parameters.
    ili9225_fill(lcd, 0xFFFF);// Call the ili9225_fill function to fill the entire screen with a white background (color value 0xFFFF). This ensures that the display starts with a clean slate before we render any text or graphics on it.
    g_lcd = lcd;// Set the global pointer g_lcd to point to the device context structure for the ILI9225 LCD. This allows us to access the LCD context from other parts of the driver, such as the file operations, where we need to perform drawing operations on the LCD based on user input.

    /* Register char device */
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret)
        return ret;

    cdev_init(&ili_cdev, &fops);// Initialize the character device structure (ili_cdev) with the file operations defined in fops. This sets up the character device to use our custom write function (ili_write) for handling write operations from user space. The cdev_init function prepares the cdev structure for registration with the kernel, allowing us to create a character device node that user space applications can interact with.
    ret = cdev_add(&ili_cdev, dev_num, 1);// Add the character device to the system using cdev_add. We pass the initialized cdev structure (ili_cdev), the device number (dev_num) that we allocated earlier, and the number of devices (1) that we want to create. If cdev_add returns a non-zero value, it indicates that there was an error during the addition of the character device, and we need to handle this error by cleaning up resources and returning the error code.
    if (ret)// Check if the character device was successfully added to the system. If cdev_add returns a non-zero value, it indicates that there was an error during the addition process (e.g., device number conflict, memory issue). In this case, we need to clean up the resources we allocated for the character device and return the error code to indicate the failure.
        goto err_chr;

    ili_class = class_create(CLASS_NAME);// Create a device class for the ILI
    if (IS_ERR(ili_class)) {// Create a device class for the ILI
        ret = PTR_ERR(ili_class);
        goto err_cdev;
    }

    if (IS_ERR(device_create(ili_class, NULL, dev_num, NULL, DEVICE_NAME))) {// Create a device node for the character device we added earlier. We pass the class we created (ili_class), a parent device (NULL), the device number (dev_num), and the name of the device (DEVICE_NAME). If device_create returns an error pointer, it indicates that there was an error during the creation of the device node (e.g., insufficient permissions, device number conflict). In this case, we need to clean up resources and return the error code to indicate the failure.
        ret = -EINVAL;
        goto err_class;
    }

    pr_info("ILI9225 Ready\n");// Print an informational message to the kernel log indicating that the ILI9225 driver is ready and has been successfully initialized. This message can be helpful for debugging and confirming that the driver has been loaded and is operational.
    return 0;

err_class:// Error handling for class creation failure: clean up cdev and unregister device number
    class_destroy(ili_class);// Destroy the device class that was created for the ILI9225 character device. This is part of the cleanup process when there is an error during the creation of the device node. By destroying the class, we ensure that any resources associated with it are released properly.
err_cdev:// Error handling for character device addition failure: unregister device number
    cdev_del(&ili_cdev);// Delete the character device that was initialized earlier. This is part of the cleanup process when there is an error during the addition of the character device. By deleting the cdev, we ensure that any resources associated with it are released properly and that it is no longer registered with the kernel.
err_chr:// Error handling for character device registration failure: unregister device number
    unregister_chrdev_region(dev_num, 1);// Unregister the device number that was allocated for the character device. This is part of the cleanup process when there is an error during the registration of the character device. By unregistering the device number, we ensure that it is released back to the system and can be reused by other drivers or devices in the future.
    return ret;
}

/* Remove callback: clean up all device/class/cdev resources */
static void ili9225_remove(struct spi_device *spi)
{
    device_destroy(ili_class, dev_num);/// Destroy the device node that was created for the ILI9225 character device. This is part of the cleanup process when the device is removed. By destroying the device, we ensure that any resources associated with it are released properly and that it is no longer accessible from user space.
    class_destroy(ili_class);// Destroy the device class that was created for the ILI9225 character device. This is part of the cleanup process when the device is removed. By destroying the class, we ensure that any resources associated with it are released properly.
    cdev_del(&ili_cdev);// Delete the character device that was initialized earlier. This is part of the cleanup process when the device is removed. By deleting the cdev, we ensure that any resources associated with it are released properly and that it is no longer registered with the kernel.
    unregister_chrdev_region(dev_num, 1);// Unregister the device number that was allocated for the character device. This is part of the cleanup process when the device is removed. By unregistering the device number, we ensure that it is released back to the system and can be reused by other drivers or devices in the future.
}

/* Device-tree match table */
static const struct of_device_id ili9225_dt_ids[] = {// Define a device tree match table for the ILI9225 driver. This table is used by the SPI core to match the driver with compatible devices defined in the device tree. Each entry in the table specifies a compatible string that corresponds to the device we want to support. In this case, we specify "ilitek,ili9225" as the compatible string, which should match the compatible property of the SPI device defined in the device tree for our LCD panel.
    { .compatible = "ilitek,ili9225" },
    {}
};
MODULE_DEVICE_TABLE(of, ili9225_dt_ids);// This macro registers the device tree match table with the kernel, allowing the SPI core to use it for matching devices with the driver based on the compatible strings defined in the device tree.

/* SPI driver registration structure */
static struct spi_driver ili9225_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = ili9225_dt_ids,
    },
    .probe = ili9225_probe,
    .remove = ili9225_remove,
};

module_spi_driver(ili9225_driver);// This macro registers the SPI driver with the kernel. It takes care of initializing the driver structure and registering it with the SPI core, allowing the driver to be matched with compatible SPI devices based on the device tree match table. When a matching device is found, the probe function will be called to initialize the device, and when the device is removed, the remove function will be called to clean up resources.

MODULE_LICENSE("GPL");// Specify the license for the kernel module. In this case, we use "GPL" to indicate that the module is licensed under the GNU General Public License. This is important for legal reasons and also allows the module to use certain symbols exported by the kernel that are only available to GPL-licensed modules.
MODULE_AUTHOR("Nandini + Project Integration");// Specify the author of the kernel module. This is a string that can include the name(s) of the developer(s) or organization responsible for creating the module. It is used for informational purposes and can help with identifying the source of the module in case of issues or contributions.
MODULE_DESCRIPTION("ILI9225 SPI TFT display char driver");// Provide a brief description of the kernel module. This string should summarize the purpose and functionality of the module. In this case, we describe it as an "ILI9225 SPI TFT display char driver" to indicate that it is a character device driver for an ILI9225 LCD display that communicates over SPI. This description can be helpful for users and developers to understand the role of the module when they see it listed in the kernel or when they check its information using tools like modinfo.
