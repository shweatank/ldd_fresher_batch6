#include <linux/gpio.h>     // GPIO control functions
#include <linux/delay.h>    // Delay functions like udelay() and msleep()

#include "dht11.h"          // Header file for DHT11/DHT22 definitions

/* ---------------- DHT READ ---------------- */

// Function to measure pulse duration from sensor
// expected -> GPIO state to wait for (0 or 1)
static int read_pulse(int expected)
{
    int t = 0;

    // Measure how long the GPIO pin stays in expected state
    while (gpio_get_value(GPIO_DHT) == expected) {

        // Delay for 1 microsecond
        udelay(1);

        // Increase pulse counter
        // If timeout occurs, return error
        if (++t >= PULSE_TIMEOUT) return -1;
    }

    // Return measured pulse length
    return t;
}


// Function to read temperature and humidity data
// temp -> stores temperature value
// hum  -> stores humidity value
int dht11_read_data(int *temp, int *hum)
{
    // Array to store 5 bytes received from sensor
    uint8_t data[5] = {0};

    int i, ht;

    // Variable used for saving interrupt flags
    unsigned long flags;

    // Set GPIO as output and keep HIGH for stabilization
    gpio_direction_output(GPIO_DHT, 1);
    udelay(10);

    // Send LOW signal for 20ms to start communication
    gpio_set_value(GPIO_DHT, 0);
    msleep(20);

    // Pull line HIGH again
    gpio_set_value(GPIO_DHT, 1);

    // Disable interrupts for accurate timing
    local_irq_save(flags);

    // Change GPIO to input mode to receive data
    gpio_direction_input(GPIO_DHT);

    // Wait 10 microseconds before reading
    udelay(10);

    /* -------- Sensor Handshake -------- */

    // Wait for sensor response pulses
    if (read_pulse(1) < 0) {
        local_irq_restore(flags);
        return -EIO;
    }

    if (read_pulse(0) < 0) {
        local_irq_restore(flags);
        return -EIO;
    }

    if (read_pulse(1) < 0) {
        local_irq_restore(flags);
        return -EIO;
    }

    /* -------- Read 40 bits from sensor -------- */

    for (i = 0; i < 40; i++) {

        // Wait for LOW pulse
        if (read_pulse(0) < 0) {
            local_irq_restore(flags);
            return -EIO;
        }

        // Measure HIGH pulse duration
        ht = read_pulse(1);

        if (ht < 0) {
            local_irq_restore(flags);
            return -EIO;
        }

        // Shift previous bits to left
        data[i / 8] <<= 1;

        // Long HIGH pulse means bit = 1
        //High pulse > 40 -> bit = 1;
        //High pulse < 40 -> bit = 0;
        if (ht > 40)
            data[i / 8] |= 1;
    }

    // Re-enable interrupts
    local_irq_restore(flags);

    /* -------- Checksum Verification -------- */

    // Verify received checksum byte
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF))
        return -EBADMSG;

    // Store humidity and temperature values
    *hum = data[0];
    *temp = data[2];

    // Return success
    return 0;
}


/* ---------------- INIT ---------------- */

// Function to initialize DHT sensor GPIO
int dht11_init()
{
    // Check whether GPIO pin is valid
    if (!gpio_is_valid(GPIO_DHT))
        return -ENODEV;

    // Request control of GPIO pin
    gpio_request(GPIO_DHT, "dht22");

    // Set GPIO as input initially
    gpio_direction_input(GPIO_DHT);

    // Print initialization message in kernel log
    printk(KERN_INFO "DHT22 driver initialized \n");

    return 0;
}


/* ---------------- EXIT ---------------- */

// Function to release GPIO resources
void dht11_exit()
{
    // Free allocated GPIO pin
    gpio_free(GPIO_DHT);

    // Print removal message in kernel log
    printk(KERN_INFO "DHT22 driver removed\n");
}
