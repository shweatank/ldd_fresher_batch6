#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>

#define DEVICE_NAME "pwm_led"
#define CLASS_NAME "pwm_class"

// PWM registers for BCM2711 (Raspberry Pi 4)
#define PWM_BASE 0xFe20c000
#define PWMCLK_BASE 0xFe1010a0

#define PWMCTL 0x00    // PWM Control register
#define PWMSTA 0x04    // PWM Status register
#define PWMDMAC 0x08   // PWM DMA Control
#define PWMRNG1 0x10   // PWM Channel 1 Range
#define PWMDAT1 0x14   // PWM Channel 1 Data (duty cycle)
#define PWMFIF1 0x18   // PWM FIFO Input
#define PWMRNG2 0x20   // PWM Channel 2 Range
#define PWMDAT2 0x24   // PWM Channel 2 Data

#define PWMCLK_CNTL 0x00  // PWM Clock Control
#define PWMCLK_DIV 0x04   // PWM Clock Divider

// GPIO base for setting pin mode
#define GPIO_BASE 0xFe200000
#define GPFSEL1 0x04  // GPIO Function Select 1 (for GPIO 10-19)

static void __iomem *pwm_base;     // Mapped PWM registers
static void __iomem *pwmclk_base;  // Mapped PWM Clock registers
static void __iomem *gpio_base;    // Mapped GPIO registers

static dev_t dev_num;              // Device number
static struct cdev pwm_cdev;       // Character device structure
static struct class *pwm_class;    // Device class
static struct device *pwm_device;  // Device structure

// File operations: open the device
static int pwm_open(struct inode *inode, struct file *file) {
    pr_info("PWM LED device opened\n");
    return 0;
}

// File operations: close the device
static int pwm_release(struct inode *inode, struct file *file) {
    pr_info("PWM LED device closed\n");
    return 0;
}

// File operations: write to the device (set PWM duty cycle)
static ssize_t pwm_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    char kbuf[16];  // Kernel buffer for user input
    int value;      // Parsed integer value (0-100)
    int duty;       // Calculated duty cycle (0-1024)

    // Limit input length to prevent overflow
    if (len > 15) len = 15;

    // Copy data from user space to kernel space
    if (copy_from_user(kbuf, buf, len)) {
        return -EFAULT;  // Error if copy fails
    }
    kbuf[len] = '\0';  // Null-terminate the string

    // Convert string to integer
    if (kstrtoint(kbuf, 10, &value)) {
        return -EINVAL;  // Invalid input
    }

    // Clamp value to 0-100 range
    if (value < 0) value = 0;
    if (value > 100) value = 100;

    // Scale value to PWM duty cycle (0-1024)
    duty = (value * 1024) / 100;

    // Write duty cycle to PWM data register
    writel(duty, pwm_base + PWMDAT1);

    pr_info("PWM duty set to %d (value %d)\n", duty, value);

    return len;  // Return number of bytes written
}

// File operations structure
static struct file_operations fops = {
    .open = pwm_open,
    .release = pwm_release,
    .write = pwm_write,
};

// Module initialization
static int __init pwm_init(void) {
    int ret;

    pr_info("Initializing PWM LED driver\n");

    // Map PWM registers into kernel virtual memory
    pwm_base = ioremap(PWM_BASE, 0x28);
    if (!pwm_base) {
        pr_err("Failed to ioremap PWM base\n");
        return -ENOMEM;
    }

    // Map PWM Clock registers
    pwmclk_base = ioremap(PWMCLK_BASE, 0x08);
    if (!pwmclk_base) {
        pr_err("Failed to ioremap PWMCLK base\n");
        iounmap(pwm_base);
        return -ENOMEM;
    }

    // Map GPIO registers
    gpio_base = ioremap(GPIO_BASE, 0xB4);
    if (!gpio_base) {
        pr_err("Failed to ioremap GPIO base\n");
        iounmap(pwm_base);
        iounmap(pwmclk_base);
        return -ENOMEM;
    }

    // Allocate a major and minor number for the device
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate char device region\n");
        goto err_iounmap;
    }

    // Initialize the character device
    cdev_init(&pwm_cdev, &fops);
    ret = cdev_add(&pwm_cdev, dev_num, 1);
    if (ret < 0) {
        pr_err("Failed to add cdev\n");
        goto err_unregister;
    }

    // Create a device class
    pwm_class = class_create(CLASS_NAME);
    if (IS_ERR(pwm_class)) {
        pr_err("Failed to create class\n");
        goto err_cdev_del;
    }

    // Create the device node in /dev/
    pwm_device = device_create(pwm_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(pwm_device)) {
        pr_err("Failed to create device\n");
        goto err_class_destroy;
    }

    // Set GPIO 18 to ALT5 mode (PWM0 function)
    // Read current GPFSEL1 register
    uint32_t gpfsel = readl(gpio_base + GPFSEL1);
    gpfsel &= ~(7 << 24);  // Clear bits 24-26 (GPIO 18 function select)
    gpfsel |= (2 << 24);   // Set to 010 (ALT5)
    writel(gpfsel, gpio_base + GPFSEL1);

    // Configure PWM Clock
    // Stop the clock first
    writel(0x5A000000 | 1, pwmclk_base + PWMCLK_CNTL);
    udelay(10);  // Small delay
    // Set divider for ~1MHz clock (assuming 19.2MHz source / 19.2 ≈ 1MHz)
    writel(0x5A000000 | (19 << 12) | 1, pwmclk_base + PWMCLK_DIV);
    // Start clock using PLLD
    writel(0x5A000011, pwmclk_base + PWMCLK_CNTL);

    // Configure PWM Channel 1
    writel(0, pwm_base + PWMCTL);     // Disable PWM
    udelay(10);
    writel(1024, pwm_base + PWMRNG1); // Set range to 1024 (for 0-100% duty)
    writel(0, pwm_base + PWMDAT1);     // Initial duty cycle 0
    writel(0x81, pwm_base + PWMCTL);   // Enable PWM1 in Mark-Space mode

    pr_info("PWM LED driver initialized successfully\n");
    return 0;

err_class_destroy:
    class_destroy(pwm_class);
err_cdev_del:
    cdev_del(&pwm_cdev);
err_unregister:
    unregister_chrdev_region(dev_num, 1);
err_iounmap:
    iounmap(gpio_base);
    iounmap(pwmclk_base);
    iounmap(pwm_base);
    return ret;
}

// Module cleanup
static void __exit pwm_exit(void) {
    pr_info("Exiting PWM LED driver\n");

    // Disable PWM
    writel(0, pwm_base + PWMCTL);
    // Stop PWM clock
    writel(0x5A000000 | 1, pwmclk_base + PWMCLK_CNTL);

    // Remove device and class
    device_destroy(pwm_class, dev_num);
    class_destroy(pwm_class);
    cdev_del(&pwm_cdev);
    unregister_chrdev_region(dev_num, 1);

    // Unmap registers
    iounmap(gpio_base);
    iounmap(pwmclk_base);
    iounmap(pwm_base);

    pr_info("PWM LED driver exited\n");
}

module_init(pwm_init);
module_exit(pwm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple PWM LED Controller for Raspberry Pi 4");
