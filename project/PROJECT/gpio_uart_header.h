/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UPDATED_H
#define UPDATED_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>

/* Names */
#define DRIVER_NAME "bcm2711_gpio_blink"
#define DEVICE_NAME "ir_sensor_dev"

/* IOCTL */
#define IOCTL_SET_VAL _IOW('a', 'a', int)

/* GPIO base physical address and size (BCM2711 / Pi 4) */
#define GPIO_BASE_PHYS  0xFE200000
#define GPIO_SIZE       0xB4

/* GPIO used for IR sensor
 * NOTE: (512+17) depends on gpiochip base on your kernel.
 */
#define GPIO_IR_SENSOR (512 + 17)

/* UART base physical address and size (PL011 on Pi 4) */
#define UART_BASE_PHYS  0xFE201400
#define UART_SIZE       0x1000

/* UART register offsets */
#define UART_DR     0x00
#define UART_FR     0x18
#define UART_IBRD   0x24
#define UART_FBRD   0x28
#define UART_LCRH   0x2C
#define UART_CR     0x30
#define UART_IMSC   0x38
#define UART_MIS    0x40
#define UART_ICR    0x44

/* UART flag bits */
#define TXFF (1 << 5)   /* Transmit FIFO full */
#define RXFE (1 << 4)   /* Receive FIFO empty */

/* UART IRQ (verify for your platform/UART instance) */
#define UART_IRQ 38

#endif /* UPDATED_H */
