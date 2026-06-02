#include<linux/module.h>
#include<linux/interrupt.h>
#include<linux/delay.h>
#include<linux/init.h>
#include<linux/ioctl.h>

#define IRQ_NUM 1
#define DEVICE_NAME "add_using_thread"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC, 1,struct Operation)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC, 1, int)

static int major;
static int kernel_value = 0;
static int ready = 0;
/*ioctl handler */
struct Operation
{
        int num1;
        int num2;
};
struct Operation user_value;

static irqreturn_t irq_top(int irq, void *dev_id)
{
	return IRQ_WAKE_THREAD;
}

static irqreturn_t irq_thread(int irq, void *dev_id)
{

	pr_info("Threaded IRQ handler\n");
	kernel_value=user_value.num1+user_value.num2;
	ready=1;
	return IRQ_HANDLED;
}

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{

        switch(cmd)
        {
                case IOCTL_SET_VALUE:

                        if(copy_from_user(&user_value,(struct Operation __user * ) arg, sizeof(struct Operation)))
                                return -EFAULT;
                        ready=0;
                        break;

                case IOCTL_GET_VALUE:

                        if(!ready)
                                return -EAGAIN;

                        if(copy_to_user((int __user *)arg, &kernel_value, sizeof(int)))
                                return -EFAULT;

                        ready=0;
                        break;

                default:
                        return -EINVAL;
        }
        return 0;
}
static struct file_operations fops =
{
        .owner = THIS_MODULE,
        .unlocked_ioctl = basic_ioctl,
};

static int __init irq_threaded_init(void)
{
	major = register_chrdev(0,DEVICE_NAME,&fops);
	int ret;
	ret=request_threaded_irq(IRQ_NUM,irq_top,irq_thread,IRQF_SHARED,"irq_threaded",(void *)irq_thread);
	
	if(ret)
	{
		pr_err("%s: Failed to request ISR %d\n",DEVICE_NAME,IRQ_NUM);
		return ret;
	}
	pr_info("basic ioctl loaded, major=%d\n",major);
	return 0;
}

static void __exit irq_threaded_exit(void)
{
	unregister_chrdev(major, DEVICE_NAME);
	free_irq(IRQ_NUM, (void *)irq_thread);
}

module_init(irq_threaded_init);
module_exit(irq_threaded_exit);

MODULE_LICENSE("GPL");
