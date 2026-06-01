#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/err.h>

/* ---------- Module Info ---------- */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prudhvi");
MODULE_DESCRIPTION("MCP3208 LDR SPI Driver");
/* ---------- GPIO Functions ---------- */
static void gpio_output(int pin);
static void gpio_write(int pin, int value);
static void led_config(void);

/* ---------- LCD Functions ---------- */
static void lcd_enable(void);
static void lcd_send4(u8 data);
void lcd_cmd(u8 cmd);
void lcd_data(u8 data);
void lcd_print(char *str);
void lcd_clear(void);
void lcd_init(void);

/* ---------- UART Functions ---------- */
static void uart_init(void);
static void uart_send_char(char c);
static void uart_send_string(const char *str);

/* ---------- SPI / ADC Functions ---------- */
static int mcp3208_read_channel(u8 channel);

/* ---------- Workqueue / Timer Functions ---------- */
static void my_work_handler(struct work_struct *work);
static void my_timer_callback(struct timer_list *t);

/* ---------- SPI Driver Probe / Remove ---------- */
static int ldr_probe(struct spi_device *spi);
static void ldr_remove(struct spi_device *spi);
/* ---------- GPIO Definitions ---------- */
#define GPIO_LED (21 + 512)   // LED GPIO pin
static bool led_gpio_requested = false; // Track if LED GPIO request succeeded

/* ---------- UART Base (memory mapped) ---------- */
#define uart_base_phys 0xFE201000
#define uart_size 1000

/* ---------- UART Registers ---------- */
#define UART_DR   0x00
#define UART_FR   0x18
#define UART_IBRD 0x24
#define UART_FBRD 0x28
#define UART_LCRH 0x2C
#define UART_CR   0x30

/* ---------- UART Flags ---------- */
#define TXFF (1 << 5)
#define RXFE (1 << 4)
#define UARTEN (1 << 0)
#define TXE    (1 << 8)
#define RXE    (1 << 9)

/* ---------- GPIO Base ---------- */
#define BCM_BASE       0xFE000000UL
#define GPIO_BASE      (BCM_BASE + 0x200000)

/* ---------- GPIO Registers ---------- */
#define GPFSEL0 0x00
#define GPFSEL1 0x04
#define GPFSEL2 0x08
#define GPSET0  0x1C
#define GPCLR0  0x28

/* ---------- LCD GPIO Pins ---------- */
#define RS 17
#define EN 27
#define D4 22
#define D5 23
#define D6 24
#define D7 25

/* ---------- Device Config ---------- */
#define DEVICE_NAME "ldr_adc"
#define TIMER_INTERVAL_MS 1000 // Timer interval in ms

/* ---------- Global Variables ---------- */
static struct spi_device *ldr_spi;
static struct timer_list my_timer;
static void __iomem *uart_base;
static void __iomem *gpio;
static struct work_struct my_work;

char buffer[32];
int flag;

/* ------------GPIO Functions ----------------------   */

/* Configure a GPIO pin as output */
static void gpio_output(int pin)
{
    int reg = pin / 10;
    int shift = (pin % 10) * 3;
    u32 val = readl(gpio + GPFSEL0 + (reg * 4));

    val &= ~(7 << shift);   // Clear bits
    val |= (1 << shift);    // Set as output

    writel(val, gpio + GPFSEL0 + (reg * 4));
}

/* Write HIGH or LOW to GPIO */
static void gpio_write(int pin, int value)
{
    if (value)
        writel(1 << pin, gpio + GPSET0);
    else
        writel(1 << pin, gpio + GPCLR0);
}

/* -------------LCD  Functions------------ */

/* Pulse the Enable pin to latch data */
static void lcd_enable(void)
{
    gpio_write(EN, 1);
    udelay(20);
    gpio_write(EN, 0);
    udelay(100);
}

/* Send 4 bits of data to LCD */
static void lcd_send4(u8 data)
{
    gpio_write(D4, (data >> 0) & 1);
    gpio_write(D5, (data >> 1) & 1);
    gpio_write(D6, (data >> 2) & 1);
    gpio_write(D7, (data >> 3) & 1);
    lcd_enable();
}

/* Send command to LCD */
void lcd_cmd(u8 cmd)
{
    gpio_write(RS, 0);
    lcd_send4(cmd >> 4);
    lcd_send4(cmd & 0x0F);
    udelay(200);
}

/* Send data (character) to LCD */
void lcd_data(u8 data)
{
    gpio_write(RS, 1);
    lcd_send4(data >> 4);
    lcd_send4(data & 0x0F);
    udelay(200);
}

/* Print string to LCD */
void lcd_print(char *str)
{
    while (*str)
        lcd_data(*str++);
}

/* Clear LCD screen */
void lcd_clear(void)
{
    lcd_cmd(0x01);
    msleep(5);
}

/* Initialize LCD in 4-bit mode */
void lcd_init(void)
{
    gpio_output(RS);
    gpio_output(EN);
    gpio_output(D4);
    gpio_output(D5);
    gpio_output(D6);
    gpio_output(D7);

    msleep(50);

    gpio_write(RS, 0);
    gpio_write(EN, 0);

    /* Initialize 4-bit mode sequence */
    lcd_send4(0x03);
    msleep(5);
    lcd_send4(0x03);
    msleep(5);
    lcd_send4(0x03);
    msleep(5);
    lcd_send4(0x02); // Set 4-bit mode
    msleep(5);

    lcd_cmd(0x28); // 2-line display, 5x8 font
    lcd_cmd(0x0C); // Display ON, cursor OFF
    lcd_cmd(0x06); // Entry mode
    lcd_clear();   // Clear display
    lcd_cmd(0x80); // Cursor home
}

/*------------------ UART Functions --------------     */

/* Initialize UART */
static void uart_init(void)
{
    writel(0x0, uart_base + UART_CR);      // Disable UART
    writel(312, uart_base + UART_IBRD);    // Integer baud rate
    writel(32, uart_base + UART_FBRD);     // Fractional baud rate
    writel((3 << 5), uart_base + UART_LCRH); // 8-bit, no parity, 1 stop
    writel(UARTEN | TXE | RXE, uart_base + UART_CR); // Enable UART
    pr_info("UART initialized\n");
}

/* Send a single character over UART */
static void uart_send_char(char c)
{
    while (readl(uart_base + UART_FR) & TXFF); // Wait until TX FIFO not full
    writel(c, uart_base + UART_DR);
}

/* Send string over UART */
static void uart_send_string(const char *str)
{
    while (*str)
        uart_send_char(*str++);
}

/*------------------ LED GPIO Safe Setup--------------------       */
static void led_config(void)
{
    if (!gpio_is_valid(GPIO_LED)) {
        pr_err("LED: Invalid GPIO %d\n", GPIO_LED);
        return;
    }

    if (gpio_request(GPIO_LED, DEVICE_NAME)) {
        pr_err("LED: Failed to request GPIO %d\n", GPIO_LED);
        return;
    }

    led_gpio_requested = true;
    gpio_direction_output(GPIO_LED, 0); // Set output, initially OFF
}

/*---------------------- MCP3208 SPI ADC Read ----------------------       */
static int mcp3208_read_channel(u8 channel)
{
    u8 tx[3], rx[3];
    struct spi_transfer t = { .tx_buf = tx, .rx_buf = rx, .len = 3 };
    struct spi_message m;

    tx[0] = 0x06 | ((channel & 0x04) >> 2);
    tx[1] = (channel & 0x03) << 6;
    tx[2] = 0x00;

    spi_message_init(&m);
    spi_message_add_tail(&t, &m);

    if (spi_sync(ldr_spi, &m) < 0) {
        pr_err("SPI transfer failed\n");
        return -EIO;
    }

    return ((rx[1] & 0x0F) << 8) | rx[2];
}

/* Workqueue Handler: Read ADC, Update LED, UART, LCD       */
static void my_work_handler(struct work_struct *work)
{
    int adc_ldr = mcp3208_read_channel(0);
    int adc_temp = mcp3208_read_channel(1);
    int ret;

    if (adc_ldr < 0 || adc_temp < 0)
        return;

    pr_info("LDR ADC Value = %d   Temperature ADC Value = %d\n",
            adc_ldr, adc_temp);

    /* LED control */
    if (led_gpio_requested) {
        if ((adc_ldr <= 100) && !flag) {
            gpio_set_value(GPIO_LED, 1);
            pr_info("LED: ON\n");
            flag = 1;
        } else if ((adc_ldr > 100) && (adc_ldr < 4096) && flag) {
            gpio_set_value(GPIO_LED, 0);
            pr_info("LED: OFF\n");
            flag = 0;
        }
    }

    /* Send values over UART */
    ret = snprintf(buffer, sizeof(buffer), "%d %d", adc_ldr, adc_temp);
    uart_send_string(buffer);

    /* Display on LCD */
    char buf1[40], buf2[40];
    int ldr = (adc_ldr * 100) / 4095;          // Scale to 0-100
    int temp_scaled = (adc_temp *300) / 4095; // Scale to 0-33.0C
    int temp_int = temp_scaled ;
   

    snprintf(buf1, sizeof(buf1), "LIGHT=%d%%", ldr);
    snprintf(buf2, sizeof(buf2), "TEMP=%dC", temp_int);

    lcd_clear();
    lcd_cmd(0x84);
    lcd_print(buf1);

    lcd_cmd(0xc0);
    lcd_print(gpio_get_value(GPIO_LED) ? "LED=ON" : "LED=OFF");

    lcd_cmd(0xc8);
    lcd_print(buf2);
}

/*------------- Timer Callback: Schedule Workqueue -----------------    */
static void my_timer_callback(struct timer_list *t)
{
    schedule_work(&my_work);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));
}

/* Device Tree and SPI Tables                                */
static const struct of_device_id ldr_dt_ids[] = {
    { .compatible = "custom,ldr-mcp3208" },
    { }
};
MODULE_DEVICE_TABLE(of, ldr_dt_ids);

static const struct spi_device_id ldr_spi_id[] = {
    { "ldr-mcp3208", 0 },
    { }
};
MODULE_DEVICE_TABLE(spi, ldr_spi_id);

/* SPI Probe: Initialize SPI, UART, GPIO, LCD, Timer        */
static int ldr_probe(struct spi_device *spi)
{
    int ret;

    pr_info("LDR SPI Driver Probe\n");

    ldr_spi = spi;
    spi->mode = SPI_MODE_0;
    spi->bits_per_word = 8;
    spi->max_speed_hz = 1000000;

    ret = spi_setup(spi);
    if (ret) {
        pr_err("SPI setup failed\n");
        return ret;
    }

    uart_base = ioremap(uart_base_phys, uart_size);
    if (!uart_base) {
        pr_err("Failed to map UART\n");
        return -ENOMEM;
    }

    uart_init();
    uart_send_string("LDR and Temperature data\n\r");

    gpio = ioremap(GPIO_BASE, 0x1000);
    if (!gpio) {
        pr_err("GPIO IOREMAP FAILED\n");
        return -ENOMEM;
    }

    lcd_init();
    led_config();             // Safe LED setup

    INIT_WORK(&my_work, my_work_handler);
    timer_setup(&my_timer, my_timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));

    pr_info("SPI: STARTED FETCHING DATA\n");
    return 0;
}

/*------------------------- SPI Remove: Clean up resources -----------------------------*/
static void ldr_remove(struct spi_device *spi)
{
    del_timer_sync(&my_timer);  // Stop timer
    flush_work(&my_work);        // Finish workqueue

    writel(0x0, uart_base + UART_CR); // Disable UART
    if (uart_base)
        iounmap(uart_base);

    lcd_clear();
    iounmap(gpio);

    if (led_gpio_requested) {
        gpio_set_value(GPIO_LED, 0);
        gpio_free(GPIO_LED);
        led_gpio_requested = false;
    }

    pr_info("LDR Driver Removed\n");
}

/*------------------- SPI Driver Structure ------------------   */
static struct spi_driver ldr_driver = {
    .driver = {
        .name = "ldr_mcp3208",
        .of_match_table = ldr_dt_ids,
    },
    .id_table = ldr_spi_id,
    .probe = ldr_probe,
    .remove = ldr_remove,
};

/*------------ Register SPI Driver------------------ */
module_spi_driver(ldr_driver);
