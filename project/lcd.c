#include <linux/module.h>
#include <linux/delay.h>

#include "gpio.h"
#include "lcd.h"

/*
 * Generate enable pulse for LCD
 * LCD reads data on EN pulse
 */
static void lcd_enable(void)
{
    gpio_write(EN, 1);

    udelay(20);

    gpio_write(EN, 0);

    udelay(100);
}

/*
 * Send 4-bit data to LCD
 * Used in 4-bit communication mode
 */
static void lcd_send4(u8 data)
{
    // Write individual bits to LCD data pins
    gpio_write(D4, (data >> 0) & 1);
    gpio_write(D5, (data >> 1) & 1);
    gpio_write(D6, (data >> 2) & 1);
    gpio_write(D7, (data >> 3) & 1);

    // Trigger LCD enable pulse
    lcd_enable();
}

/*
 * Send command to LCD
 * RS = 0 indicates command mode
 */
void lcd_cmd(u8 cmd)
{
    gpio_write(RS, 0);

    // Send higher 4 bits
    lcd_send4(cmd >> 4);

    // Send lower 4 bits
    lcd_send4(cmd & 0x0F);

    udelay(200);
}

/*
 * Send character/data to LCD
 * RS = 1 indicates data mode
 */
void lcd_data(u8 data)
{
    gpio_write(RS, 1);

    // Send higher 4 bits
    lcd_send4(data >> 4);

    // Send lower 4 bits
    lcd_send4(data & 0x0F);

    udelay(200);
}

/*
 * Print string on LCD
 */
void lcd_print(char *s)
{
    while (*s)
        lcd_data(*s++);
}

/*
 * Initialize LCD in 4-bit mode
 */
void lcd_init(void)
{
    // Configure LCD control and data pins as output
    gpio_output(RS);

    gpio_output(EN);

    gpio_output(D4);

    gpio_output(D5);

    gpio_output(D6);

    gpio_output(D7);

    // Wait for LCD power stabilization
    msleep(50);

    /*
     * LCD initialization sequence
     * Switch LCD from default 8-bit mode to 4-bit mode
     */
    lcd_send4(0x03);

    msleep(5);

    lcd_send4(0x03);

    msleep(5);

    lcd_send4(0x03);

    msleep(5);

    lcd_send4(0x02);

    /*
     * LCD configuration commands
     */

    // 4-bit mode, 2-line display, 5x8 font
    lcd_cmd(0x28);

    // Display ON, cursor OFF
    lcd_cmd(0x0C);

    // Auto increment cursor
    lcd_cmd(0x06);

    // Clear display
    lcd_cmd(0x01);

    // Return cursor to home position
    lcd_cmd(0x02);

    msleep(5);
}
