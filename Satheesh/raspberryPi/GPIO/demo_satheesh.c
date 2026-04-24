#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/timer.h>

#define DEVICE_NAME "led_gpio"
#define CLASS_NAME  "led"

#define GPIO_LED    17
#define GPIO_BUTTON 18

static int major;
static struct class *led_class = NULL;
static struct device *led_device = NULL;

static int irq_number;
static int led_state = 0;
static struct timer_list led_timer;

/* ---------------- TIMER ---------------- */
static void led_timer_func(struct timer_list *t)
{
    gpio_set_value(GPIO_LED, 0);
    led_state = 0;
    pr_info("LED: OFF (timer)\n");
}

/* ---------------- IRQ ---------------- */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    gpio_set_value(GPIO_LED, 1);
    led_state = 1;
    pr_info("LED: ON (IRQ)\n");

    mod_timer(&led_timer, jiffies + msecs_to_jiffies(300));

    return IRQ_HANDLED;
}

/* ---------------- FILE OPS ---------------- */
static int led_open(struct inode *inodep, struct file *filep)
{
    return 0;
}

static int led_release(struct inode *inodep, struct file *filep)
{
    return 0;
}

static ssize_t led_write(struct file *filep,
                         const char __user *buffer,
                         size_t len, loff_t *offset)
{
    char msg;

    if (copy_from_user(&msg, buffer, 1))
        return -EFAULT;

    if (msg == '1')
        gpio_set_value(GPIO_LED, 1);
    else if (msg == '0')
        gpio_set_value(GPIO_LED, 0);
    else
        return -EINVAL;

    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
    .release = led_release,
};

/* ---------------- INIT ---------------- */
static int __init led_init(void)
{
    int ret;

    pr_info("Driver init\n");

    /* Request GPIOs */
    ret = gpio_request(GPIO_LED, "LED");
    if (ret)
        return ret;

    ret = gpio_request(GPIO_BUTTON, "BUTTON");
    if (ret) {
        gpio_free(GPIO_LED);
        return ret;
    }

    gpio_direction_output(GPIO_LED, 0);
    gpio_direction_input(GPIO_BUTTON);

    /* IRQ */
    irq_number = gpio_to_irq(GPIO_BUTTON);
    if (irq_number < 0) {
        gpio_free(GPIO_LED);
        gpio_free(GPIO_BUTTON);
        return irq_number;
    }

    ret = request_irq(irq_number,
                      gpio_irq_handler,
                      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                      "gpio_irq",
                      NULL);
    if (ret) {
        gpio_free(GPIO_LED);
        gpio_free(GPIO_BUTTON);
        return ret;
    }

    /* Timer */
    timer_setup(&led_timer, led_timer_func, 0);

    /* Char device */
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        free_irq(irq_number, NULL);
        gpio_free(GPIO_LED);
        gpio_free(GPIO_BUTTON);
        return major;
    }

    led_class = class_create(CLASS_NAME);
    led_device = device_create(led_class, NULL,
                               MKDEV(major, 0), NULL, DEVICE_NAME);

    pr_info("Loaded: /dev/%s\n", DEVICE_NAME);
    return 0;
}

/* ---------------- EXIT ---------------- */
static void __exit led_exit(void)
{
    del_timer_sync(&led_timer);

    gpio_set_value(GPIO_LED, 0);

    free_irq(irq_number, NULL);

    gpio_free(GPIO_LED);
    gpio_free(GPIO_BUTTON);

    device_destroy(led_class, MKDEV(major, 0));
    class_destroy(led_class);
    unregister_chrdev(major, DEVICE_NAME);

    pr_info("Driver removed\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
