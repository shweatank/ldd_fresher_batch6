#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define DRIVER_NAME "led_gpio_driver_intr"
#define DEVICE_NAME "led_gpio"
#define CLASS_NAME  "led"

// GPIO pins (BCM numbering)
#define GPIO_LED (17+512)
#define GPIO_BUTTON (18+512)

static int major;
static struct class*  led_class  = NULL;
static struct device* led_device = NULL;

static int irq_number;
static int led_state = 0;

// IRQ handler: toggles the LED
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    led_state ^= 1;  // toggle LED state
    gpio_set_value(GPIO_LED, led_state);
    pr_info("LED: toggled to %d\n", led_state);
    return IRQ_HANDLED;
}

// Character device operations
static int led_open(struct inode *inodep, struct file *filep)
{
    pr_info("LED: device opened\n");
    return 0;
}

static int led_release(struct inode *inodep, struct file *filep)
{
    pr_info("LED: device closed\n");
    return 0;
}

static ssize_t led_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    char msg;

    if (len < 1)
        return -EINVAL;

    if (copy_from_user(&msg, buffer, 1))
        return -EFAULT;

    if (msg == '1') {
        led_state = 1;
        gpio_set_value(GPIO_LED, 1);
        pr_info("LED: ON via write\n");
    } else if (msg == '0') {
        led_state = 0;
        gpio_set_value(GPIO_LED, 0);
        pr_info("LED: OFF via write\n");
    } else {
        return -EINVAL;
    }

    return 1;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
    .release = led_release,
};

// Module initialization
static int __init led_init(void)
{
    int ret;

    pr_info("LED: initializing driver...\n");

    // Validate GPIOs
    if (!gpio_is_valid(GPIO_LED) || !gpio_is_valid(GPIO_BUTTON)) {
        pr_err("LED: invalid GPIOs\n");
        return -ENODEV;
    }

    // Request GPIOs
    ret = gpio_request(GPIO_LED, "LED_GPIO");
    if (ret) {
        pr_err("LED: failed to request LED GPIO\n");
        return ret;
    }

    ret = gpio_request(GPIO_BUTTON, "BUTTON_GPIO");
    if (ret) {
        pr_err("LED: failed to request BUTTON GPIO\n");
        gpio_free(GPIO_LED);
        return ret;
    }

    // Set directions
    gpio_direction_output(GPIO_LED, 0);   // LED off initially
    gpio_direction_input(GPIO_BUTTON);

    // Map button GPIO to IRQ
    irq_number = gpio_to_irq(GPIO_BUTTON);
    if (irq_number < 0) {
        pr_err("LED: failed to map GPIO to IRQ\n");
        gpio_free(GPIO_LED);
        gpio_free(GPIO_BUTTON);
        return irq_number;
    }

    // Request IRQ (toggle on rising and falling edge)
    ret = request_irq(irq_number,
                      gpio_irq_handler,
                      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                      "gpio_button_irq",
                      NULL);
    if (ret) {
        pr_err("LED: failed to request IRQ\n");
        gpio_free(GPIO_LED);
        gpio_free(GPIO_BUTTON);
        return ret;
    }

    // Register character device
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_err("LED: failed to register char device\n");
        free_irq(irq_number, NULL);
        gpio_free(GPIO_LED);
        gpio_free(GPIO_BUTTON);
        return major;
    }

    // Create device class (updated for kernel 6.x)
    led_class = class_create(CLASS_NAME);
    if (IS_ERR(led_class)) {
        pr_err("LED: failed to create class\n");
        unregister_chrdev(major, DEVICE_NAME);
        free_irq(irq_number, NULL);
        gpio_free(GPIO_LED);
        gpio_free(GPIO_BUTTON);
        return PTR_ERR(led_class);
    }

    // Create device node /dev/led_gpio
    led_device = device_create(led_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(led_device)) {
        pr_err("LED: failed to create device\n");
        class_destroy(led_class);
        unregister_chrdev(major, DEVICE_NAME);
        free_irq(irq_number, NULL);
        gpio_free(GPIO_LED);
        gpio_free(GPIO_BUTTON);
        return PTR_ERR(led_device);
    }

    pr_info("LED: driver loaded successfully. Use /dev/%s\n", DEVICE_NAME);
    return 0;
}

// Module cleanup
static void __exit led_exit(void)
{
    pr_info("LED: unloading driver...\n");

    // Turn off LED
    gpio_set_value(GPIO_LED, 0);

    // Free IRQ
    free_irq(irq_number, NULL);

    // Free GPIOs
    gpio_free(GPIO_LED);
    gpio_free(GPIO_BUTTON);

    // Remove device node and class
    device_destroy(led_class, MKDEV(major, 0));
    class_unregister(led_class);
    class_destroy(led_class);

    // Unregister char device
    unregister_chrdev(major, DEVICE_NAME);

    pr_info("LED: driver unloaded\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("GPIO LED driver with interrupt (fixed for kernel 6.x)");
