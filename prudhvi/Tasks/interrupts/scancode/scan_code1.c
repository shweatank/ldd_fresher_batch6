#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/io.h>
#include <linux/uaccess.h>
#define DEVICE_NAME "basic_ioctl_drv" 
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,struct num)

#define KBD_IRQ 1
#define KBD_DATA_PORT 0X60
static int major;
static int kernel_add=0;
static int kernel_mul=0;
static int kernel_sub=0;
struct num
{
	int a,b;
};

struct num k;
static irqreturn_t keyboard_interrupt(int irq,void *dev_id)
{
	unsigned char scancode;

	scancode =inb(KBD_DATA_PORT);


			if(scancode==0x9E)
{
	printk(KERN_INFO"keyboard IRQ:SCAN CODE =0x%x \n",scancode);
	printk(KERN_INFO"Add=%d \n",kernel_add);
}
			else if(scancode==0x9F)
{
	printk(KERN_INFO"keyboard IRQ:SCAN CODE =0x%x \n",scancode);
	printk(KERN_INFO"sub=%d \n",kernel_sub);
}
			else if(scancode==0xB2)
{
	printk(KERN_INFO"keyboard IRQ:SCAN CODE =0x%x \n",scancode);
	printk(KERN_INFO"mul=%d \n",kernel_mul);
}
	return IRQ_HANDLED;

}
static long basic_ioctl(struct file *file ,unsigned int cmd,unsigned long arg)
{

	struct num u;

	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			if(copy_from_user(&u,(struct num __user *)arg,sizeof(u)))
				return -EFAULT;

			pr_info("kernel:recevied %d %d  from user\n",u.a,u.b);
			k.a=u.a;
			k.b=u.b;
				kernel_add=k.a+k.b;
				kernel_sub=k.a-k.b;
				kernel_mul=k.a*k.b;
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static struct file_operations fops = {

	.owner	=THIS_MODULE,
	.unlocked_ioctl	=basic_ioctl,
};

static int __init kbd_driver_init(void)
{
	int result;
	printk(KERN_INFO"keyboard driver loaded\n");

	result =request_irq(KBD_IRQ,
			keyboard_interrupt,
			IRQF_SHARED,
			"kbd_driver",
			(void*)(keyboard_interrupt));
	if(result)
	{
		printk(KERN_ERR"connot register IRQ 1\n");
		return result;
	}
	major=register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("basic_ioctl loaded,major=%d\n",major);

	return 0;

}


static void __exit kbd_driver_exit(void)
{
	free_irq(KBD_IRQ,(void*)(keyboard_interrupt));
	printk(KERN_INFO"keboard brover unloadded\n");
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("basic_ioctl unloaded\n");


}
module_init(kbd_driver_init);
module_exit(kbd_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PRUDHVI");
MODULE_DESCRIPTION("SIMPLE KEYBOARD INTERRUPT DRIVER");
