#ifndef GPIO_H
#define GPIO_H

#include <linux/io.h>

// GPIO virtual base address pointer
extern void __iomem *gpio;

/*
 * Base address of BCM SoC peripherals
 * Used in Raspberry Pi 4
 */
#define BCM_BASE       0xFE000000UL

/*
 * GPIO controller base address
 */
#define GPIO_BASE      (BCM_BASE + 0x200000)

/*
 * GPIO Function Select Registers
 * Used to configure GPIO pin mode
 * (input/output/alternate functions)
 */
#define GPFSEL0        0x00
#define GPFSEL1        0x04
#define GPFSEL2        0x08

/*
 * GPIO Pin Output Set Register
 * Writing 1 sets corresponding GPIO HIGH
 */
#define GPSET0         0x1C

/*
 * GPIO Pin Output Clear Register
 * Writing 1 sets corresponding GPIO LOW
 */
#define GPCLR0         0x28

/*
 * Configure GPIO pin as output
 */
void gpio_output(int pin);

/*
 * Write value to GPIO pin
 * value = 1 -> HIGH
 * value = 0 -> LOW
 */
void gpio_write(int pin, int value);

#endif
