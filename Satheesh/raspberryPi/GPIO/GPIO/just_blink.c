#include<linux/module.h>
#include<linux/init.h>
#include<linux/gpio.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>
#include<linux/delay.h>

//#include<linux/module.h>

#define DRIVER_NAME "Basic_led_blink"
#define DEVICE_NAME "led_gpio"

#define CLASS_NAME "my_led"
 #define GPIO_LED (26+512)

static int major;
static struct class * led_class=NULL;
static struct  device* led_device=NULL;

static int __init my_init(void)
{
	pr_info("initialising led\n");
	if(!gpio_is_valid(GPIO_LED))
	{
		pr_err("LED: invalid gpio %d\n",GPIO_LED);
		return -ENODEV;
	}
	gpio_request(GPIO_LED,DRIVER_NAME);
	gpio_direction_output(GPIO_LED,0);
	//major=register_chrdev(0,DEVICE_NAME,&fops);
	while(1)
	{
		msleep(1000);
		gpio_set_value(GPIO_LED,1);
		msleep(1000);
		gpio_set_value(GPIO_LED,0);
	}
}
static void __exit my_exit(void)
{
	gpio_set_value(GPIO_LED,0);
	gpio_free(GPIO_LED);
}
module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("Simply writing bro !");

