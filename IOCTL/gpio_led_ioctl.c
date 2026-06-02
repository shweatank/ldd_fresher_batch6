#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>        // For GPIO functions
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/init.h>

#define DRIVER_NAME "led_gpio_ioctl"
#define DEVICE_NAME "led_gpio_ioctl"
#define CLASS_NAME  "led"

#define MAGIC_NUMBER 'B'
#define SET_PIN_STATUS _IOW(MAGIC_NUMBER,1,int)
#define GET_PIN_STATUS _IOR(MAGIC_NUMBER,2,int)

// GPIO17 BCM = GPIO_BASE + 17 = 512 + 17 for Raspberry Pi 4
#define GPIO_LED (17 + 512)

static int major;
static struct class*  led_class  = NULL;
static struct device* led_device = NULL;

static int led_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "LED: Device opened\n");
    return 0;
}

static int led_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "LED: Device closed\n");
    return 0;
}

static long basic_ioctl(struct file *file,unsigned int cmd,
                unsigned long arg)
{
        int user_value=0;

        switch(cmd)
        {
                case SET_PIN_STATUS:
                        /*copy status from user */
                        if(copy_from_user(&user_value,(int __user*)arg,sizeof(int)))
                                return -EFAULT;

                        printk(KERN_INFO "kernel:received %d from user\n",user_value);

                        gpio_set_value(GPIO_LED,user_value);

                        
                          break;

                 case GET_PIN_STATUS:

                          int status=gpio_get_value(GPIO_LED);
			  printk(KERN_INFO "Get pin called\n");
                        /*copy data from user */
                        if(copy_to_user((int __user*)arg,&status,sizeof(int)))
                                return -EFAULT;

                        break;
                 default:printk(KERN_INFO "DEFAULT CASE CALLED\n");
			return -EFAULT;
        }
        return 0;
}
                                                                              

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .unlocked_ioctl=basic_ioctl,
    .release = led_release,
};

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
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        gpio_free(GPIO_LED);
        pr_err("LED: Failed to register major number\n");
        return major;
    }

    led_class = class_create(CLASS_NAME);
    if (IS_ERR(led_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        gpio_free(GPIO_LED);
        pr_err("LED: Failed to create class\n");
        return PTR_ERR(led_class);
    }

    led_device = device_create(led_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(led_device)) {
        class_destroy(led_class);
        unregister_chrdev(major, DEVICE_NAME);
        gpio_free(GPIO_LED);
        pr_err("LED: Failed to create device\n");
        return PTR_ERR(led_device);
    }

    pr_info("LED: Driver loaded. Use /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit led_exit(void) {
    gpio_set_value(GPIO_LED, 0);  // Turn off LED
    gpio_free(GPIO_LED);

    device_destroy(led_class, MKDEV(major, 0));
    class_unregister(led_class);
    class_destroy(led_class);
    unregister_chrdev(major, DEVICE_NAME);

    pr_info("LED: Driver unloaded\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("GPIO LED Driver without Device Tree");
