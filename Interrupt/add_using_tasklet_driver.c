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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ram");
MODULE_DESCRIPTION("Keyboard IRQ+ Tasklet (Linux 6.8 correct API)");

//Forward Declaration - New signature 
static void keyboard_tasklet_fn(struct tasklet_struct *t);

//Tasklet object
DECLARE_TASKLET(keyboard_tasklet, keyboard_tasklet_fn);

//Tasklet Bottom half

static void keyboard_tasklet_fn(struct tasklet_struct *t)
{
	kernel_value=user_value.num1+user_value.num2;
        pr_info("tasklet : bottom half executed\n");
	pr_info("tasklet : Result is %d\n",kernel_value);
	//ready=1;
}

//IRQ handler(Top Half)
static irqreturn_t keyboard_irq_handler(int irq,void *dev_id)
{
        pr_info("irq: keyboard interrupt occured\n");

        tasklet_schedule(&keyboard_tasklet);

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
	
        ret=request_irq(KEYBOARD_IRQ,keyboard_irq_handler,IRQF_SHARED,"kbd_tasklet",(void *)keyboard_irq_handler);
        if(ret)
        {
                pr_err("Failed to request IRQ %d\n",KEYBOARD_IRQ);
                return ret;
        }

        pr_info("tasklet module loaded\n");
        return 0;
}

//Module exit
static void __exit tasklet_irq_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
        tasklet_kill(&keyboard_tasklet);
        free_irq(KEYBOARD_IRQ,(void *)keyboard_irq_handler);

        pr_info("tasklet module unloaded\n");
}

module_init(tasklet_irq_init);
module_exit(tasklet_irq_exit);

