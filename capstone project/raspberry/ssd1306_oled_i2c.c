#include <linux/i2c.h>      // Linux I2C subsystem functions
#include <linux/fs.h>       // File system related functions
#include <linux/uaccess.h>  // User space memory access functions
#include <linux/delay.h>    // Delay functions like msleep()

#include "ssd1306_oled_i2c.h" // OLED driver header file


// I2C address of SSD1306 OLED display
#define OLED_ADDR 0x3C

/*
 * OLED modules sometimes show address as 0x78 on PCB.
 * Linux I2C uses 7-bit addressing, so:
 *
 * 0x78 >> 1 = 0x3C
 *
 * Therefore actual usable address is 0x3C.
 */


// Global pointer for OLED I2C client device
static struct i2c_client *client;


/* ---------------- I2C SEND FUNCTIONS ---------------- */

/*
 * Send command byte to OLED controller
 * Commands control OLED settings and cursor position
 */
static void oled_cmd(u8 cmd)
{
    // First byte 0x00 indicates command mode
    u8 buf[2] = {0x00, cmd};

    // Send command over I2C
    i2c_master_send(client, buf, 2);
}


/*
 * Send display data byte to OLED controller
 * Data bytes represent actual pixels on screen
 */
static void oled_dat(u8 data)
{
    // First byte 0x40 indicates data mode
    u8 buf[2] = {0x40, data};

    // Send pixel data over I2C
    i2c_master_send(client, buf, 2);
}


/* ---------------- OLED INITIALIZATION ---------------- */

/*
 * Configure SSD1306 OLED controller
 * These commands are mandatory hardware setup commands
 */
static void oled_init_display(void)
{
    // Turn display OFF during configuration
    oled_cmd(0xAE);

    // Set display clock divide ratio
    oled_cmd(0xD5);
    oled_cmd(0x80);

    // Set multiplex ratio for 128x32 display
    oled_cmd(0xA8);
    oled_cmd(0x1F);

    // Set display offset
    oled_cmd(0xD3);
    oled_cmd(0x00);

    // Set start line address
    oled_cmd(0x40);

    // Enable internal charge pump
    oled_cmd(0x8D);
    oled_cmd(0x14);

    // Set horizontal addressing mode
    oled_cmd(0x20);
    oled_cmd(0x00);

    // Flip display horizontally
    oled_cmd(0xA1);

    // Flip display vertically
    oled_cmd(0xC8);

    // Configure COM pins hardware
    oled_cmd(0xDA);
    oled_cmd(0x02);

    // Set display contrast/brightness
    oled_cmd(0x81);
    oled_cmd(0x8F);

    // Set pre-charge period
    oled_cmd(0xD9);
    oled_cmd(0xF1);

    // Set VCOMH deselect level
    oled_cmd(0xDB);
    oled_cmd(0x40);

    // Resume display from RAM content
    oled_cmd(0xA4);

    // Set normal display mode
    oled_cmd(0xA6);

    // Turn OLED display ON
    oled_cmd(0xAF);
}


/* ---------------- CLEAR OLED SCREEN ---------------- */

/*
 * Clear entire OLED display by writing zeros
 */
static void oled_clear(void)
{
    int i;

    // Set column address range
    oled_cmd(0x21);
    oled_cmd(0x00);
    oled_cmd(0x7F);

    // Set page address range
    oled_cmd(0x22);
    oled_cmd(0x00);
    oled_cmd(0x03);

    // Fill display memory with zeros
    for (i = 0; i < 512; i++)
        oled_dat(0x00);
}


/* ---------------- PRINT SINGLE CHARACTER ---------------- */

/*
 * Display one ASCII character using 5x7 font
 */
static void oled_print_char(char c)
{
    int i;

    // Replace unsupported characters with space
    if (c < 32 || c > 122)
        c = ' ';

    // Send 5 font bytes for character
    for (i = 0; i < 5; i++)
        oled_dat(font5x7[(u8)(c - 32)][i]);

    // Add one blank column for spacing
    oled_dat(0x00);
}


/* ---------------- SET CURSOR POSITION ---------------- */

/*
 * Set OLED cursor position
 * col  -> horizontal pixel column
 * page -> display page (row section)
 */
static void oled_set_cursor(u8 col, u8 page)
{
    // Set column range
    oled_cmd(0x21);
    oled_cmd(col);
    oled_cmd(0x7F);

    // Set page range
    oled_cmd(0x22);
    oled_cmd(page);
    oled_cmd(0x03);
}


/* ---------------- PRINT STRING ---------------- */

/*
 * Print string at selected OLED position
 */
static void oled_print_str(u8 col, u8 page, const char *s)
{
    // Set cursor position
    oled_set_cursor(col, page);

    // Print characters one by one
    while (*s)
        oled_print_char(*s++);
}


/* ---------------- DISPLAY SENSOR DATA ---------------- */

/*
 * Display temperature and humidity values
 * Also shows error messages if sensor fails
 */
void oled_display_sensor(int temp, int hum, int err)
{
    char l0[22], l1[22];

    // Clear old display contents
    oled_clear();

    // Sensor timeout error
    if (err == -EIO) {

        oled_print_str(0, 0, "DHT11: Timeout");
        oled_print_str(0, 1, "Check wiring!");

    }
    // Checksum error
    else if (err == -EBADMSG) {

        oled_print_str(0, 0, "DHT11: Checksum");
        oled_print_str(0, 1, "err,retrying..");

    }
    // Display valid sensor readings
    else {

        // Format humidity string
        snprintf(l0, sizeof(l0),
                 "Humidity:  %3d %%",
                 hum);

        // Format temperature string
        snprintf(l1, sizeof(l1),
                 "Temp:      %3d C",
                 temp);

        // Print humidity
        oled_print_str(0, 0, l0);

        // Print temperature
        oled_print_str(0, 1, l1);
    }
}


/* ---------------- MODULE INIT ---------------- */

/*
 * Initialize OLED display and I2C communication
 */
int oled_init(void)
{
    struct i2c_adapter *adapter;

    // Get I2C bus adapter (I2C-1 on Raspberry Pi)
    adapter = i2c_get_adapter(1);

    // Check adapter availability
    if (!adapter)
        return -ENODEV;

    // Create dummy I2C client for OLED device
    client = i2c_new_dummy_device(adapter, OLED_ADDR);

    // Release adapter reference
    i2c_put_adapter(adapter);

    // Check for I2C device creation errors
    if (IS_ERR(client))
        return PTR_ERR(client);

    // Initialize OLED hardware
    oled_init_display();

    // Small delay for stabilization
    msleep(100);

    // Clear OLED screen
    oled_clear();

    // Kernel log message
    pr_info("OLED char driver init\n");

    return 0;
}


/* ---------------- MODULE EXIT ---------------- */

/*
 * Cleanup OLED driver resources
 */
void oled_exit(void)
{
    // Remove I2C device
    i2c_unregister_device(client);

    // Clear display before exit
    oled_clear();

    // Kernel log message
    pr_info("OLED driver removed\n");
}
