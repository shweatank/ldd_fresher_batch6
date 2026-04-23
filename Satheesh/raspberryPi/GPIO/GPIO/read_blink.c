#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/gpio.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>

#define DRIVER_NAME "LED_READ"
#define DEVICE_NAME "led_gpio"
#define CLASS_NAME "led"

#define LED (26+512)

static int major;
static struct class* led_class=NULL;
static struct device* led_device=NULL;
static ssize_t led_write(struct file*file,const char __user*buffer,size_t len,loff_t *offset)
{
	char msg[2];
	
	if(copy_from_user(msg,buffer,len))
		return -EFAULT;
	if(msg[0]=='0')
	{
		gpio_set_value(LED,0);
		pr_info("LED:Off\n");
	}	
	else if(msg[0]=='1')
	{
		gpio_set_value(LED,1);
		pr_info("LED:On\n");
	}
	return len;
}
static struct file_operations fops={.owner=THIS_MODULE,.write=led_write};
static int __init led_init(void)
{
	if(!gpio_is_valid(LED))
	{
		pr_err("LED: Invlaid GPIO %d\n",LED);
		return -EBUSY;
	}
	if(gpio_request(LED,DRIVER_NAME))
	{
		pr_err("LED: Failed to request GPIO %d\n",LED);
		return -EBUSY;
	}
	gpio_direction_output(LED,0);
	//Setting as output initially as low 
	major=register_chrdev(0,DEVICE_NAME,&fops);
	if(major<0)
	{
		gpio_free(LED);
			pr_err("LED: Failed to register\n");
			return major;
	}
	led_class=class_create(CLASS_NAME);
	if(IS_ERR(led_class))
	{
		unregister_chrdev(major,DEVICE_NAME);
		gpio_free(LED);
		pr_err("LED:Failed to create class\n");
		return  PTR_ERR(led_class);
	}
	led_device=device_create(led_class,NULL,MKDEV(major,0),NULL,DEVICE_NAME);
	if(IS_ERR(led_device))
	{
		class_destroy(led_class);
		unregister_chrdev(major,DEVICE_NAME);
		gpio_free(LED);
		pr_err("LED: Failed to create device\n");
		return PTR_ERR(led_device);
	}
	pr_info("LED: Driver loaded\n");
	return 0;
}
static void __exit led_exit(void)
{
	gpio_set_value(LED,0);
	gpio_free(LED);
	
	device_destroy(led_class,MKDEV(major,0));
	class_unregister(led_class);
	class_destroy(led_class);
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("LED: Driver Unloaded\n");

}
module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("GPIO LED Driver");

