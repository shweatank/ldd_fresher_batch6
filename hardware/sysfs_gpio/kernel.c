#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>        // For GPIO functions
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/init.h>

#define DRIVER_NAME "led_gpio_driver"
#define DEVICE_NAME "led_gpio"
#define CLASS_NAME  "led"

//static int status = 0;

// GPIO17 BCM = GPIO_BASE + 17 = 512 + 17 for Raspberry Pi 4
#define GPIO_LED (17 + 512)

static int major;
static struct class*  led_class  = NULL;
static struct device* led_device = NULL;




/*static ssize_t led_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    char msg[2] = {0};

    if (len > 1)
        len = 1;

    if (copy_from_user(msg, buffer, len))
        return -EFAULT;

    if (msg[0] == '1') {
        gpio_set_value(GPIO_LED, 1);
        pr_info("LED: ON\n");
    } else if (msg[0] == '0') {
        gpio_set_value(GPIO_LED, 0);
        pr_info("LED: OFF\n");
    }

    return len;
}*/

static ssize_t value_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
	int get =gpio_get_value(GPIO_LED);
        return sprintf(buf,"%d\n",get);
}


static ssize_t value_store(struct kobject *kobj,struct kobj_attribute *attr,
                const char *buf, size_t count)
{
        //sscanf(buf, "%d,%d,%c",&val1,&val2,&op);
        //return count;
	
	if(buf[0] =='1'){
		gpio_set_value(GPIO_LED, 1);
		pr_info("LED : ON\n");
	}
	else if (buf[0] =='0')
	{
		gpio_set_value(GPIO_LED,0);
		pr_info("LED : ON\n");
	}
}

static struct kobj_attribute value_attr=
        __ATTR(value, 0664,value_show, value_store);




static int __init led_init(void) {

    pr_info("LED: Initializing driver...\n");

    // Request the GPIO
    if (!gpio_is_valid(GPIO_LED)) {
        pr_err("LED: Invalid GPIO %d\n", GPIO_LED);
        return -ENODEV;
    }

    if (gpio_request(GPIO_LED, DRIVER_NAME)) {
        pr_err("LED: Failed to request GPIO %d\n", GPIO_LED);
        return -EBUSY;
    }

    gpio_direction_output(GPIO_LED, 0);  // Set as output, initial LOW

    // Character device registration

        demo_kobj = kobject_create_and_add("sysfs_demo",kernel_kobj);
        if(!demo_kobj)
            return -ENOMEM;

        sysfs_create_file(demo_kobj,&value_attr.attr);
        pr_info("sysfs_demo loaded\n");
        return 0;

    pr_info("LED: Driver loaded. Use /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit led_exit(void) {
    gpio_set_value(GPIO_LED, 0);  // Turn off LED
    gpio_free(GPIO_LED);
    pr_info("LED: Driver unloaded\n");

    kobject_put(demo_kobj);
     pr_info("sysfs_demo unloaded\n");

}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("GPIO LED Driver without Device Tree");
