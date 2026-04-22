#include <linux/module.h>      // module_init, module_exit
#include <linux/kernel.h>      // pr_info, pr_err
#include <linux/init.h>        // __init, __exit

#include <linux/gpio.h>        // gpio_request, gpio_set_value, gpio_to_irq
#include <linux/interrupt.h>   // request_irq, free_irq, IRQF_TRIGGER_*

#include <linux/fs.h>          // register_chrdev
#include <linux/uaccess.h>     // copy_from_user
#include <linux/device.h>      // class_create, device_create

#define DRIVER_NAME "led_gpio_driver_intr"
#define DEVICE_NAME "led_gpio"
#define CLASS_NAME  "led"
static int irq_number;
// GPIO17 BCM = GPIO_BASE + 17 = 512 + 17 for Raspberry Pi 4
#define GPIO_LED (17 + 512)
#define GPIO_READ (18+512)
static int major;
static struct class*  led_class  = NULL;
static struct device* led_device = NULL;
static int flg=0;
static irqreturn_t gpio_irq_handler(int irq,void * dev_id)
{
	if(!flg)
	{
		gpio_set_value(GPIO_LED,1);
		flg^=1;
	}else
	{
		gpio_set_value(GPIO_LED,0);
		flg^=1;
	}
	return IRQ_HANDLED;
}
static int led_open(struct inode *inodep, struct file *filep) {
    pr_info("LED: Device opened\n");
    return 0;
}

static int led_release(struct inode *inodep, struct file *filep) {
    pr_info("LED: Device closed\n");
    return 0;
}

static ssize_t led_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    char msg[2] = {0};

    if (len > 1)
        len = 1;

    if (copy_from_user(msg, buffer, len))
        return -EFAULT;

    if (msg[0] == '1') {
        //gpio_set_value(GPIO_READ, 1);
        pr_info("LED: ON\n");
    } else if (msg[0] == '0') {
        //gpio_set_value(GPIO_READ, 0);
        pr_info("LED: OFF\n");
    }

    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
    .release = led_release,
};

static int __init led_init(void) {
int ret;
    pr_info("LED: Initializing driver...\n");

    // Request the GPIO
    if (!gpio_is_valid(GPIO_LED)) {
        pr_err("LED: Invalid GPIO %d\n", GPIO_LED);
        return -ENODEV;
    }
    if(!gpio_is_valid(GPIO_READ))
    {
	pr_err("Led: Invalid Gpio %d\n",GPIO_READ);
	return -ENODEV;
    }

    if (gpio_request(GPIO_LED, DRIVER_NAME)) {
        pr_err("LED: Failed to request GPIO %d\n", GPIO_LED);
        return -EBUSY;
    }
    if(gpio_request(GPIO_READ,DRIVER_NAME))
    {
	pr_err("LED: Failed to request GPIO %d\n",GPIO_READ);
	return -EBUSY;
    }

    gpio_direction_output(GPIO_LED, 0);  // Set as output, initial LOW
	gpio_direction_input(GPIO_READ);
    	//converting gpio to irq
    	irq_number=gpio_to_irq(GPIO_READ);
    	if(irq_number<0)
    	{
    		pr_err("Failed to create irq_number\n");
    		return irq_number;
    	}
    	ret=request_irq(irq_number,gpio_irq_handler,IRQF_TRIGGER_RISING,"GPIO18_intr",NULL);
    	if (ret) {
    pr_err("Failed to request irq\n");
    gpio_free(GPIO_LED);
    gpio_free(GPIO_READ);
    return ret;
}
    	if(irq_number<0)
    	{
		pr_err("Failed to request irq\n");
		return ret;
    	}
    // Character device registration
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        gpio_free(GPIO_LED);
        gpio_free(GPIO_READ);
        pr_err("LED: Failed to register major number\n");
        return major;
    }

    led_class = class_create(CLASS_NAME);
    if (IS_ERR(led_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        gpio_free(GPIO_LED);
        gpio_free(GPIO_READ);
        pr_err("LED: Failed to create class\n");
        return PTR_ERR(led_class);
    }

    led_device = device_create(led_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(led_device)) {
        class_destroy(led_class);
        unregister_chrdev(major, DEVICE_NAME);
        gpio_free(GPIO_LED);
        gpio_free(GPIO_READ);
        pr_err("LED: Failed to create device\n");
        return PTR_ERR(led_device);
    }

    pr_info("LED: Driver loaded. Use /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit led_exit(void) {
    gpio_set_value(GPIO_LED, 0);  // Turn off LED
    gpio_free(GPIO_LED);
    gpio_free(GPIO_READ);
	free_irq(irq_number,NULL);
	
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
