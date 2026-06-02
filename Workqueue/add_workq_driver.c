#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>

#define KEYBOARD_IRQ 1
#define DEVICE_NAME "add_using_tasklet"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC, 1, struct Operation)
//#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC, 1, int)

static int major;
static int kernel_value = 0;
//static int ready = 0;

struct Operation
{
	int num1;
	int num2;
};
struct Operation user_value;
static struct work_struct my_work;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ram");
MODULE_DESCRIPTION("Keyboard IRQ+ Tasklet (Linux 6.8 correct API)");

//WorkQueue Handler function

static void my_work_handler(struct work_struct *work)
{
	kernel_value=user_value.num1+user_value.num2;
        pr_info("Workqueue : Handler Started\n");

	pr_info("Workqueue : Addition is %d\n",kernel_value);

	pr_info("Workqueue : Handler Finished\n");
	//ready=1;
}

//IRQ handler(Top Half)
static irqreturn_t keyboard_irq_handler(int irq,void *dev_id)
{
        pr_info("irq: keyboard interrupt occured\n");

        pr_info("Workqueue: Scheduling work\n");
	schedule_work(&my_work);

        return IRQ_HANDLED;
}
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			if(copy_from_user(&user_value,(struct Operation __user * ) arg, sizeof(struct Operation)))
				return -EFAULT;
			//ready=0;
			break;
/*		case IOCTL_GET_VALUE:
			if(!ready)
				return -EAGAIN;

			if(copy_to_user((int __user *)arg, &kernel_value, sizeof(int)))
				return -EFAULT;
			ready=0;
			break;      
*/
		default:
			return -EINVAL;
	}
	return 0;
}
static struct file_operations fops = 
{
	.owner=THIS_MODULE,
	.unlocked_ioctl = basic_ioctl,
};
//Module Init
static int __init tasklet_irq_init(void)
{
	major=register_chrdev(0,DEVICE_NAME,&fops);
        int ret;
	pr_info("Workqueue: module loaded\n");

	//Initialize work
	INIT_WORK(&my_work, my_work_handler);
	pr_info("Workqueue: Scheduling work\n");
        ret=request_irq(KEYBOARD_IRQ,keyboard_irq_handler,IRQF_SHARED,"kbd_tasklet",(void *)keyboard_irq_handler);
        if(ret)
        {
                pr_err("Failed to request IRQ %d\n",KEYBOARD_IRQ);
                return ret;
        }
        return 0;
}

//Module exit
static void __exit tasklet_irq_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
        flush_work(&my_work);
        free_irq(KEYBOARD_IRQ,(void *)keyboard_irq_handler);

        pr_info("Workqueue: module unloaded\n");
}

module_init(tasklet_irq_init);
module_exit(tasklet_irq_exit);

