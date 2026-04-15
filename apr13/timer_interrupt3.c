#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

struct cal
{
	int n1;
	int n2;
	int res;
	char op;
};

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_ADD_VALUE _IOWR(IOCTL_MAGIC,1,struct cal)
#define IOCTL_SET_SUB_VALUE _IOWR(IOCTL_MAGIC,2,struct cal)
#define IOCTL_SET_MUL_VALUE _IOWR(IOCTL_MAGIC,3,struct cal)
#define IOCTL_SET_DIV_VALUE _IOWR(IOCTL_MAGIC,4,struct cal)
#define IRQ_NUM 1

static int major;


static irqreturn_t irq_top(int irq,void *dev_id)
{
	return IRQ_WAKE_THREAD;
}

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct cal data;
	switch(cmd)
	{
		case IOCTL_SET_ADD_VALUE:
			/*copy data from user*/
			if(copy_from_user(&data, (struct cal __user*)arg,sizeof(data)))
			{
				return -EFAULT;
			}
			/*kernel modidfies data*/
                         data.res=data.n1+data.n2;
			/*copy data back to user*/
			if(copy_to_user((struct cal __user*)arg,&data,sizeof(data)))
			{
				return -EFAULT;
			}
			break;
		case IOCTL_SET_SUB_VALUE:
			if(copy_from_user(&data,(struct cal __user*)arg,sizeof(data)))
			{
				return -EFAULT;
			}
			data.res=data.n1-data.n2;
			if(copy_to_user((struct cal __user*)arg,&data,sizeof(data)))
			{
				return -EFAULT;
			}
			break;
		case IOCTL_SET_MUL_VALUE:
			if(copy_from_user(&data,(struct cal __user*)arg,sizeof(data)))
			{
				return -EFAULT;
			}
			data.res=data.n1*data.n2;
			if(copy_to_user((struct cal __user*)arg,&data,sizeof(data)))
			{
				return -EFAULT;
			}
			break;
		case IOCTL_SET_DIV_VALUE:
			if(copy_from_user(&data,(struct cal __user*)arg,sizeof(data)))
			{
				return -EFAULT;
			}
			data.res=data.n1/data.n2;
			if(copy_to_user((struct cal __user*)arg,&data,sizeof(data)))
			{
				return -EFAULT;
			}
			break;
		default:
			return -EINVAL;
	}
	return 0;
}


static irqreturn_t irq_thread(int irq,void *dev_id)
{
	pr_info("Threaded IRQ handler (can sleep)\n");
	msleep(50);
	return IRQ_HANDLED;
}

static struct file_operations fops={
	.owner=THIS_MODULE,
	.unlocked_ioctl=basic_ioctl,
};


static int __init irq_threaded_init(void)
{
	major=register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("Basic_ioctl loaded,major=%d\n",major);

	return request_threaded_irq(IRQ_NUM,irq_top,irq_thread,IRQF_SHARED,"irq threaded",(void *)irq_thread);
}

static void __exit irq_threaded_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("basic_ioctl unloaded\n");

	free_irq(IRQ_NUM,(void *)irq_thread);
}

module_init(irq_threaded_init);
module_exit(irq_threaded_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pavani");
MODULE_DESCRIPTION("simple threaded interrupt handler");

