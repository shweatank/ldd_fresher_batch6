#include <linux/module.h>     
#include <linux/kernel.h>     
#include <linux/init.h>       
#include <linux/io.h>         
#include <linux/delay.h>      
#include <linux/kthread.h>    
#include <linux/string.h>     

#include "gpio.h"             // GPIO functions/macros
#include "uart.h"             // UART functions
#include "lcd.h"              // LCD functions
#include "rtc.h"              // RTC/I2C functions
#include "spi_adc.h"          // SPI ADC functions

// Kernel thread handle
static struct task_struct *monitor_task;

// Monitoring thread function
static int monitor_fn(void *d)
{
    // RTC data buffer
    u8 rtc[7];

    // LCD buffers
    char lcd1[17];
    char lcd2[17];

    // UART message buffer
    char uartbuf[64];

    // AM/PM string
    char ampm[3];

    // Time variables
    int sec, min, hr;

    // ADC variables
    int adc, mv, temp;

    // Run continuously until thread stopped
    while (!kthread_should_stop()) {

        // Read RTC through I2C
        if (ds1307_read(rtc) < 0) {

            // Print error log
            printk(KERN_ERR "RTC READ FAILED\n");

            // Wait 1 second
            msleep(1000);

            // Retry
            continue;
        }

        // Convert BCD seconds to decimal
        sec = bcd2bin(rtc[0] & 0x7F);

        // Convert BCD minutes to decimal
        min = bcd2bin(rtc[1]);

        // Check 12-hour mode
        if (rtc[2] & 0x40) {

            // Extract hour
            hr = bcd2bin(rtc[2] & 0x1F);

            // Check AM/PM
            strcpy(ampm,
                  (rtc[2] & 0x20)
                  ? "PM" : "AM");

        } else {

            // 24-hour mode
            hr = bcd2bin(rtc[2] & 0x3F);

            strcpy(ampm, "AM");
        }

        // Read ADC value through SPI
        adc = mcp3208_read();

	// Convert ADC value to millivolts using 3.3 v
        mv = (adc * 3300) / 4095;

        // Convert voltage to temperature 
	//10mv = 1 degree
        temp = mv / 10;

        // Create LCD time string
        snprintf(lcd1,sizeof(lcd1),"%02d:%02d:%02d%s",hr, min, sec, ampm);

        // Create LCD temperature string
        snprintf(lcd2,sizeof(lcd2),"TEMP %d C",temp);

        // Clear LCD first line
        lcd_cmd(0x80);
        lcd_print("                ");

        // Clear LCD second line
        lcd_cmd(0xC0);
        lcd_print("                ");

        // Display time
        lcd_cmd(0x80);
        lcd_print(lcd1);

        // Display temperature
        lcd_cmd(0xC0);
        lcd_print(lcd2);

        // Create UART message
        snprintf(uartbuf,sizeof(uartbuf),"TIME %02d:%02d:%02d %s TEMP %d C\r\n",hr, min, sec,ampm, temp);

        // Send through UART
        uart_puts(uartbuf);

        // Print kernel log
        printk(KERN_INFO "%s", uartbuf);

        // Delay 1 second
        msleep(1000);
    }

    return 0;
}

// Driver initialization
static int __init drv_init(void)
{
    // Driver start message
    printk(KERN_INFO "DRIVER START\n");

    // Map GPIO registers
    gpio = ioremap(GPIO_BASE, 0x1000);

    // Map SPI registers
    spi  = ioremap(SPI0_BASE, 0x100);

    // Map UART registers
    uart = ioremap(UART0_BASE, 0x100);

    // Map I2C registers
    i2c  = ioremap(BSC1_BASE, 0x100);

    // Check mapping failure
    if (!gpio || !spi || !uart || !i2c) {

        printk(KERN_ERR "IOREMAP FAILED\n");

        return -ENOMEM;
    }

    // Initialize UART GPIO
    uart_gpio_init();

    // Initialize UART
    uart_init();

    // Initialize I2C GPIO
    i2c_gpio_init();

    // Initialize SPI GPIO
    spi_gpio_init();

    // Initialize SPI
    spi_init();

    // Initialize LCD
    lcd_init();

    // Clear I2C FIFO
    writel(BSC_CLEAR,
           i2c + BSC_C);

    // Clear I2C status
    writel(0,
           i2c + BSC_S);

    // Set I2C clock divider
    writel(1500,
           i2c + BSC_DIV);

    // Enable I2C
    writel(BSC_I2CEN,
           i2c + BSC_C);

    // LCD startup message
    lcd_cmd(0x80);
    lcd_print("SYSTEM START");

    // UART startup message
    uart_puts("SYSTEM START\r\n");

    // Kernel startup log
    printk(KERN_INFO "SYSTEM START\n");

    // Delay 2 seconds
    msleep(2000);

    // Start monitoring thread
    monitor_task =
        kthread_run(monitor_fn,
                    NULL,
                    "monitor");

    return 0;
}

// Driver cleanup
static void __exit drv_exit(void)
{
    // Stop monitoring thread
    if (monitor_task)
        kthread_stop(monitor_task);

    // LCD exit message
    lcd_cmd(0x80);
    lcd_print("DRIVER EXIT    ");

    // UART exit message
    uart_puts("DRIVER EXIT\r\n");

    // Kernel exit log
    printk(KERN_INFO "DRIVER EXIT\n");

    // Unmap GPIO
    iounmap(gpio);

    // Unmap SPI
    iounmap(spi);

    // Unmap UART
    iounmap(uart);

    // Unmap I2C
    iounmap(i2c);
}

// Driver entry point
module_init(drv_init);

// Driver exit point
module_exit(drv_exit);

// Module license
MODULE_LICENSE("GPL");

// Module author
MODULE_AUTHOR("Project");

// Module description
MODULE_DESCRIPTION("RTC SPI ADC LCD UART Driver");
