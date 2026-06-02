#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>        // For GPIO functions
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/init.h>
#include<linux/sysfs.h>

#define DRIVER_NAME "led_gpio_sysfs"
#define DEVICE_NAME "gpio_led_sysfs"
#define CLASS_NAME  "led"

// GPIO17 BCM = GPIO_BASE + 17 = 512 + 17 for Raspberry Pi 4
#define GPIO_LED (17 + 512)


                                                                              
static struct kobject *demo_kobj;
static int demo_value;

/* Show function */

static ssize_t value_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
	printk(KERN_INFO "CURRENT STATUS=%d\n",demo_value);
        return sprintf(buf,"%d\n",demo_value);
}

/* Store function */

static ssize_t value_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{
        sscanf(buf,"%d",&demo_value);
	printk(KERN_INFO "SET VALUE=%d\n",demo_value);
	gpio_set_value(GPIO_LED,demo_value);
        return count;
}

static struct kobj_attribute value_attr =
    __ATTR(value,0664,value_show,value_store);



static int __init led_init(void) {
    pr_info("LED: Initializing driver...\n");


    if (!gpio_is_valid(GPIO_LED)) {
        pr_err("LED: Invalid GPIO %d\n", GPIO_LED);
        return -ENODEV;
    }

    if (gpio_request(GPIO_LED, DRIVER_NAME)) {
        pr_err("LED: Failed to request GPIO %d\n", GPIO_LED);
        return -EBUSY;
    }

    gpio_direction_output(GPIO_LED, 0);  // Set as output, initial LOW


    demo_kobj = kobject_create_and_add("gpio_led_sysfs",kernel_kobj);
        if(!demo_kobj)
                return -ENOMEM;

        int ret=sysfs_create_file(demo_kobj,&value_attr.attr);
        if(ret)
        {
                return ret;
        }
        pr_info("sysfs_demo loaded\n");

    pr_info("LED: Driver loaded. Use /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit led_exit(void) {
    gpio_set_value(GPIO_LED, 0);  // Turn off LED
    gpio_free(GPIO_LED);

    sysfs_remove_file(demo_kobj,&value_attr.attr);
        kobject_put(demo_kobj);
   
    pr_info("LED: Driver unloaded\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("GPIO LED Driver without Device Tree");
