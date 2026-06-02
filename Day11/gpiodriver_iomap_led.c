#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#define GPIO_BASE_PHYS 0xFE200000  // Raspberry Pi 4
#define GPIO_SIZE      0xB0        // Enough to cover needed registers

#define GPIO_PIN 18                // GPIO 18 (pin 12)

// Register offsets (in words, not bytes)
#define GPFSEL1 1   // GPIO 10-19 function select
#define GPSET0  7   // GPIO 0-31 set
#define GPCLR0 10   // GPIO 0-31 clear

static void __iomem *gpio_base;
static struct task_struct *blink_thread;
static bool blink_thread_stop = false;

// Thread function to blink LED
static int blink_fn(void *data)
{
    while (!kthread_should_stop()) {
        // Turn LED on
        writel(1 << GPIO_PIN, gpio_base + GPSET0 * 4);
        msleep(500);

        // Turn LED off
        writel(1 << GPIO_PIN, gpio_base + GPCLR0 * 4);
        msleep(500);
    }
    return 0;
}

static int __init led_init(void)
{
    unsigned int reg;

    printk(KERN_INFO "LED Blink Module Init\n");

    // Map GPIO registers
    gpio_base = ioremap(GPIO_BASE_PHYS, GPIO_SIZE);
    if (!gpio_base) {
        printk(KERN_ERR "Failed to map GPIO memory\n");
        return -ENOMEM;
    }

    // Set GPIO 18 as output
    reg = readl(gpio_base + GPFSEL1 * 4);
    reg &= ~(7 << 24);   // Clear bits 24-26 for GPIO18
    reg |=  (1 << 24);   // Set as output
    writel(reg, gpio_base + GPFSEL1 * 4);

    // Start blinking thread
    blink_thread_stop = false;
    blink_thread = kthread_run(blink_fn, NULL, "led_blink_thread");
    if (IS_ERR(blink_thread)) {
        printk(KERN_ERR "Failed to create blink thread\n");
        iounmap(gpio_base);
        return PTR_ERR(blink_thread);
    }

    return 0;
}

static void __exit led_exit(void)
{
    printk(KERN_INFO "LED Blink Module Exit\n");

    // Stop thread
    if (blink_thread) {
        kthread_stop(blink_thread);
    }

    // Turn off LED
    writel(1 << GPIO_PIN, gpio_base + GPCLR0 * 4);

    // Unmap GPIO memory
    if (gpio_base)
        iounmap(gpio_base);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("Blink LED on Raspberry Pi 4 using ioremap");
