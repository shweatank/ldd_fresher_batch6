#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/gpio/consumer.h>

#define DRIVER_NAME "led_gpio_driver_intr"
#define DEVICE_NAME "led_gpio"
#define CLASS_NAME  "led"

// GPIO pins (BCM numbering)
#define GPIO_LED    (26+512)
#define GPIO_BUTTON (02+512)

static int major;
static struct class*  led_class  = NULL;
static struct device* led_device = NULL;

static int irq_number;
static int led_state = 0;
static struct timer_list led_timer;
//struct gpio_desc *gpio_desc_button = NULL;

// Timer function: turn LED off
static void led_timer_func(struct timer_list *t)
{
    gpio_set_value(GPIO_LED, 0);
    led_state = 0;
    pr_info("LED: OFF (timer)\n");
}

// IRQ handler: turn LED ON and start timer to turn off
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    gpio_set_value(GPIO_LED, 1);  // Turn LED ON
    led_state = 1;
    pr_info("LED: ON (IRQ)\n");

    // Schedule timer to turn off LED after 300ms
    mod_timer(&led_timer, jiffies + msecs_to_jiffies(300));

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
        gpio_set_value(GPIO_LED, 1);
        led_state = 1;
        pr_info("LED: ON via write\n");
    } else if (msg == '0') {
        gpio_set_value(GPIO_LED, 0);
        led_state = 0;
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

    // Request IRQ (rising edge, button press)
    ret = request_irq(irq_number,
                      gpio_irq_handler,
                      IRQF_TRIGGER_RISING,
                      "gpio_button_irq",
                      NULL);
    if (ret) {
        pr_err("LED: failed to request IRQ\n");
        gpio_free(GPIO_LED);
        gpio_free(GPIO_BUTTON);
        return ret;
    }

    // Setup debounce (200ms)
//    gpio_desc_button = gpio_to_desc(GPIO_BUTTON);
  //  if (gpio_desc_button)
    //    gpiod_set_debounce(gpio_desc_button, 200);
    //else
      //  pr_warn("LED: failed to set debounce\n");

    // Setup timer for LED off
    timer_setup(&led_timer, led_timer_func, 0);

    // Register character device
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_err("LED: failed to register char device\n");
        free_irq(irq_number, NULL);
        gpio_free(GPIO_LED);
        gpio_free(GPIO_BUTTON);
        return major;
    }

    // Create device class
    led_class = class_create(CLASS_NAME);
    if (IS_ERR(led_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        free_irq(irq_number, NULL);
        gpio_free(GPIO_LED);
        gpio_free(GPIO_BUTTON);
        return PTR_ERR(led_class);
    }

    // Create device node /dev/led_gpio
    led_device = device_create(led_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(led_device)) {
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

    del_timer_sync(&led_timer);   // Remove timer
    gpio_set_value(GPIO_LED, 0);   // Turn off LED
    free_irq(irq_number, NULL);    // Free IRQ
    gpio_free(GPIO_LED);           // Free GPIOs
    gpio_free(GPIO_BUTTON);

    device_destroy(led_class, MKDEV(major, 0));
    class_unregister(led_class);
    class_destroy(led_class);
    unregister_chrdev(major, DEVICE_NAME);

    pr_info("LED: driver unloaded\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("GPIO LED driver with interrupt and timer (kernel 6.x)");
