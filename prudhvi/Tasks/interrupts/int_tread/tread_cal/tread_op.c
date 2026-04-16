#include<linux/module.h>
#include<linux/interrupt.h>
#include<linux/delay.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/string.h>
#include<linux/io.h>

struct data 
{
	int a,b;
}k;
#define IRQ_NUM 1
#define KBD_DATA_PORT 0X60
#define DEVICE_NAME "THREAD_CAL" 
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,struct data)
static int major;
static int k_add=0,k_sub=0,k_mul;
static irqreturn_t irq_top(int irq,void *dev_id)
{
	return IRQ_WAKE_THREAD;
}

static irqreturn_t irq_thread(int irq,void *dev_id)
{
	unsigned char scancode;
	scancode=inb(KBD_DATA_PORT);
	if(scancode==0x9E)
	{
		printk(KERN_INFO"keyboard IRQ:SCAN CODE =0x%x \n",scancode);
		printk(KERN_INFO"Add=%d \n",k_add);
	}
	else if(scancode==0x9F)
	{
		printk(KERN_INFO"keyboard IRQ:SCAN CODE =0x%x \n",scancode);
		printk(KERN_INFO"sub=%d \n",k_sub);
	}
	else if(scancode==0xB2)
	{
		printk(KERN_INFO"keyboard IRQ:SCAN CODE =0x%x \n",scancode);
		printk(KERN_INFO"mul=%d \n",k_mul);
	}
	return IRQ_HANDLED;
}
static long basic_ioctl(struct file *file ,unsigned int cmd,unsigned long arg)
{

	struct data u;

	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			if(copy_from_user(&u,(struct data __user *)arg,sizeof(u)))
				return -EFAULT;

			pr_info("kernel:recevied  %d %d  from user\n",u.a,u.b);

			k.a=u.a;
			k.b=u.b;
			k_add=k.a+k.b;
			k_sub=k.a-k.b;
			k_mul=k.a*k.b;

			break;
		default:
			return -EINVAL;
	}
	return 0;
}


static struct file_operations fops = {

	.owner  =THIS_MODULE,
	.unlocked_ioctl =basic_ioctl,
};

static int __init irq_thread_init(void)
{
	major=register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("basic_ioctl loaded,major=%d\n",major);
	return request_threaded_irq(IRQ_NUM,
			irq_top,
			irq_thread,
			IRQF_SHARED,
			"irq_threaded",
			(void*)irq_thread);
}

static void __exit irq_thread_exit(void)
{
unregister_chrdev(major,DEVICE_NAME);
         pr_info("basic_ioctl unloaded\n");

	free_irq(IRQ_NUM, (void*)irq_thread);
}

module_init(irq_thread_init);
module_exit(irq_thread_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("prudhvi");

MODULE_DESCRIPTION("interrupt using thread");
