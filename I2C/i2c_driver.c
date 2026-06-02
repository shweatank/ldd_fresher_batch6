//bcm2711_gpio_uart.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define DRIVER_NAME "bcm2711_uart"
#define DEVICE_NAME "i2c"
#define CLASS_NAME "i2c"

//BCM2711 GPIO base
#define GPIO_BASE     0xFE200000
#define I2C_BASE      0x7E804000
#define I2C_SIZE      0x1000

//Register offsets
#define I2C_C    	 0x00
#define I2C_S    	 0x04
#define I2C_DLEN 	 0x08
#define I2C_A    	 0x0C
#define I2C_FIFO	 0x10
#define I2C_DIV		 0x14
#define I2C_DEL 	 0x18
#define I2C_CLKT         0x1C

static void __iomem *i2c_base;
static void __iomem *gpio_base;
static int major;
static struct class* i2c_class = NULL;
static struct device* i2c_device = NULL;

static int i2c_open(struct inode *inodep , struct file *filep)
{
	pr_info("I2C : I2C Started\n");
	return 0;
}

static int i2c_release(struct inode *inodep, struct file *filep)
{
	pr_info("I2C : I2C Completed\n");
	return 0;
}

static void i2c_init(void)
{




