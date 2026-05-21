#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>
#include <linux/gpio.h>        // For GPIO functions
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/init.h>
#define GPIO_LED (21 + 512)

static struct kobject *demo_kobj;

// show function
static ssize_t value_show(struct kobject *kobj,
                         struct kobj_attribute *attr,
                         char *buf)
{
int get_value;
get_value=gpio_get_value(GPIO_LED);

    return sprintf(buf, "%d\n", get_value);
}

// store function
static ssize_t value_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf,
		size_t count)
{
	int set_value;

	sscanf(buf,"%d", &set_value);
	if(set_value==0)
	{

		gpio_set_value(GPIO_LED,0);
		pr_info("Led=OFF\n");
	}
	else if(set_value==1)
	{

		gpio_set_value(GPIO_LED,1);
		pr_info("Led=ON\n");
	}
	return count;
}

static struct kobj_attribute value_attr =
    __ATTR(gpio, 0664, value_show, value_store);

static int __init sysfs_demo_init(void)
{
if (!gpio_is_valid(GPIO_LED)) {
        pr_err("LED: Invalid GPIO %d\n", GPIO_LED);
        return -ENODEV;
    }

    if (gpio_request(GPIO_LED, "sysfs_gpio_demo")) {
        pr_err("LED: Failed to request GPIO %d\n", GPIO_LED);
        return -EBUSY;
    }

    gpio_direction_output(GPIO_LED, 0);  // Set as output, initial LOW


    demo_kobj = kobject_create_and_add("sysfs_gpio_demo", kernel_kobj);

    if (!demo_kobj)
        return -ENOMEM;

    if (sysfs_create_file(demo_kobj, &value_attr.attr)) {
        kobject_put(demo_kobj);
        return -ENOMEM;
    }

    pr_info("sysfs_gpio_demo loaded\n");
    return 0;
}

static void __exit sysfs_demo_exit(void)
{
gpio_set_value(GPIO_LED, 0);  // Turn off LED
    gpio_free(GPIO_LED);

    kobject_put(demo_kobj);
    pr_info("sysfs_demo unloaded\n");
}

module_init(sysfs_demo_init);
module_exit(sysfs_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prudhvi");
