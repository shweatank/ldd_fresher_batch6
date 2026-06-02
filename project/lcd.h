#ifndef LCD_H
#define LCD_H

#include <linux/types.h>

/*
 * LCD Control Pins
 */

// Register Select pin
// RS = 0 -> Command mode
// RS = 1 -> Data mode
#define RS 17

// Enable pin
// Used to latch data into LCD
#define EN 27

/*
 * LCD Data Pins (4-bit mode)
 */
#define D4 22
#define D5 23
#define D6 24
#define D7 25

/*
 * Initialize LCD
 * Configures GPIOs and LCD settings
 */
void lcd_init(void);

/*
 * Send command to LCD
 */
void lcd_cmd(u8 cmd);

/*
 * Send single character/data to LCD
 */
void lcd_data(u8 data);

/*
 * Print string on LCD
 */
void lcd_print(char *s);

#endif
