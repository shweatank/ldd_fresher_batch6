#include<linux/init.h>
#include<linux/module.h>
#include<linux/gpio.h>
#include<linux/uaccess.h>
#include<linux/fs.h>
#include<linux/ioctl.h>
#define MAGIC_NUMBER 'B'
#define LED_DATA _IOW(MAGIC_NUMBER,1,int)
#define DEVICE_NAME  "ioclt_led"
#define LED (21+512)

static int major;
static long my_ioctl(struct file*file,unsigned int cmd,unsigned long arg)
{

	if(cmd==LED_DATA)
	{
		int ch;
		if(copy_from_user(&ch,(char __user*)arg,sizeof(int)))
		{
			pr_err("LED: Can't write from user\n");
			return -ENOMEM;
		}
		pr_info("LED:Received %d\n",ch);

		if(ch==1){
			gpio_set_value(LED,1);
			pr_info("LED:ON\n");
			}
		if(ch==0){
			gpio_set_value(LED,0);
			
			pr_info("LED:OFF\n");
			}
	}
	return 0;
}
static struct file_operations fops={
	.owner=THIS_MODULE,
	.unlocked_ioctl=my_ioctl
};
static int __init my_init(void)
{
	if(!gpio_is_valid(LED))
	{
		pr_err("LED:Invalid led\n");
		return -ENODEV;
	}	
	if(gpio_request(LED,DEVICE_NAME))
	{
		pr_err("LED:Failed to request\n");
		return -EBUSY;
	}
	gpio_direction_output(LED,0);
	major=register_chrdev(0,DEVICE_NAME,&fops);
	if(major<0)
	{
		gpio_free(LED);
		pr_err("LED:Failed to register major number\n");

	}
		pr_info("LED: ioctl_led major=%d\n",major);
	return 0;
}
static void __exit my_exit(void)
{
	gpio_set_value(LED,0);
	gpio_free(LED);
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("LED:Driver unloaded\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("The Great Satheesh");
MODULE_DESCRIPTION("Own Led module");










