#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/io.h>
#include<linux/uaccess.h>
#include<linux/delay.h>
struct two
{
	int a,b;
}kernel_value;
#define DEVICE_NAME "intr_cal"
#define MAGIC_NUMBER 'B'
#define SND_NUM _IOW(MAGIC_NUMBER,1,struct two)
static int major;
#define KBD_IRQ 1
#define KBD_DATA_PORT 0x60
static long my_ioctl(struct file*file,unsigned int cmd,unsigned long arg)
{
	if(cmd==SND_NUM)
	{
		if(copy_from_user(&kernel_value,(struct two __user*)arg,sizeof(struct two)))
	return -EFAULT;
	}
return 0;
}
static  irqreturn_t keyboard_interrupt(int irq,void *dev_id)
{
	unsigned char scancode;
	//read scan code from keyboard port 
	scancode=inb(KBD_DATA_PORT);
	//printk(KERN_INFO"KEyboard IRQ:scan code=0x%x\n",scancode);
	msleep(1);
	switch (scancode)
	{
		case 0x9e:
			printk(KERN_INFO"sum:%d\n",kernel_value.a+kernel_value.b);
			break;
		case 0x9f:
			printk(KERN_INFO"sub:%d\n",kernel_value.a-kernel_value.b);
			break;
		case 0xb2:
			printk(KERN_INFO"mul:%d\n",kernel_value.a*kernel_value.b);
			break;
		case 0x90:
			printk(KERN_INFO"sum:%d\n",kernel_value.a/kernel_value.b);
			break;

	}
	return IRQ_HANDLED;
}
static struct file_operations fops={.owner=THIS_MODULE,.unlocked_ioctl=my_ioctl};

static int __init kbd_driver_init(void)
{
	int result;
	printk(KERN_INFO"keyboard driver loaded\n");
//Request IRQ 1 (keyboard
result =request_irq(KBD_IRQ,keyboard_interrupt,IRQF_SHARED,"kbd_driver",(void*)(keyboard_interrupt));
if(result)
{
	printk(KERN_ERR"Cannot register IRQ\n");
	return result;
}
	major=register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("%s loaded major=%d\n",DEVICE_NAME,major);	
return 0;
}
static void __exit kbd_driver_exit(void)
{
	free_irq(KBD_IRQ,(void*)(keyboard_interrupt));
	printk(KERN_INFO"Keyboard Driver Unloaded\n");
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("Device unloaded\n");
}
module_init(kbd_driver_init);
module_exit(kbd_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("Simple keyboard interrupt driver");








