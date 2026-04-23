/* LED blinking using sysfs */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#define DRIVER_NAME "led_gpio_sysfs"

// GPIO17 BCM = 512 + 17 (Raspberry Pi 4)
#define GPIO_LED (17 + 512)

static struct kobject *led_kobj;
static int led_value = 0;

/* Show function (read from sysfs) */
static ssize_t led_show(struct kobject *kobj,
                        struct kobj_attribute *attr,
                        char *buf)
{
    return sprintf(buf, "%d\n", led_value);
}

/* Store function (write to sysfs) */
static ssize_t led_store(struct kobject *kobj,
                         struct kobj_attribute *attr,
                         const char *buf, size_t count)
{
    if (buf[0] == '1') {
        gpio_set_value(GPIO_LED, 1);
        led_value = 1;
        pr_info("LED: ON\n");
    }
    else if (buf[0] == '0') {
        gpio_set_value(GPIO_LED, 0);
        led_value = 0;
        pr_info("LED: OFF\n");
    }

    return count;
}

/* Attribute */
static struct kobj_attribute led_attr =
    __ATTR(led_value, 0664, led_show, led_store);

/* Init function */
static int __init led_init(void)
{
    int ret;

    pr_info("LED SYSFS: Initializing...\n");

    /* Validate GPIO */
    if (!gpio_is_valid(GPIO_LED)) {
        pr_err("Invalid GPIO\n");
        return -ENODEV;
    }

    /* Request GPIO */
    ret = gpio_request(GPIO_LED, DRIVER_NAME);
    if (ret) {
        pr_err("GPIO request failed\n");
        return ret;
    }

    gpio_direction_output(GPIO_LED, 0);

    /* Create sysfs directory */
    led_kobj = kobject_create_and_add("led_gpio", kernel_kobj);
    if (!led_kobj) {
        gpio_free(GPIO_LED);
        return -ENOMEM;
    }

    /* Create sysfs file */
    ret = sysfs_create_file(led_kobj, &led_attr.attr);
    if (ret) {
        kobject_put(led_kobj);
        gpio_free(GPIO_LED);
        return ret;
    }

    pr_info("LED SYSFS: Created /sys/kernel/led_gpio/led_value\n");
    return 0;
}

/* Exit function */
static void __exit led_exit(void)
{
    gpio_set_value(GPIO_LED, 0);

    sysfs_remove_file(led_kobj, &led_attr.attr);
    kobject_put(led_kobj);

    gpio_free(GPIO_LED);

    pr_info("LED SYSFS: Driver unloaded\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("GPIO LED Driver using SYSFS");
