#include <linux/module.h>
#include "gpio.h"

// Pointer to mapped GPIO register base address
void __iomem *gpio;

/*
 * Configure a GPIO pin as output
 */
void gpio_output(int pin)
{
    // Each GPFSEL register controls 10 GPIO pins
    int reg = pin / 10;

    // Each pin uses 3 bits in the function select register
    int shift = (pin % 10) * 3;

    u32 val;

    // Read current value of the function select register
    val = readl(gpio + GPFSEL0 + reg * 4);

    // Clear the 3 bits corresponding to the pin
    val &= ~(7 << shift);

    // Set pin function to output (001)
    val |= (1 << shift);

    // Write updated value back to register
    writel(val, gpio + GPFSEL0 + reg * 4);
}

/*
 * Set GPIO pin HIGH or LOW
 */
void gpio_write(int pin, int value)
{
    if (value)
        // Set pin HIGH using GPSET register
        writel(1 << pin, gpio + GPSET0);
    else
        // Set pin LOW using GPCLR register
        writel(1 << pin, gpio + GPCLR0);
}
