#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/io.h>
#include<linux/delay.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/wait.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'G'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,int)


#define IRQ_NUM 1

#define KEYBOARD_IRQ 1
MODULE_LICENSE("GPL");
MODULE_AUTHOR("GANGADHAR");
MODULE_DESCRIPTION("Simple Linux Kernel Keyboard Interrupt Driver");
#define KBD_DATA_PORT 0x60


static wait_queue_head_t wq;
static int flag = 0;

static int major;
//static int kernel_value = 0;

static int num1 = 0,num2 = 20,res = 0;

//forward declaration _ new signature
static void keyboard_tasklet_fn(struct tasklet_struct *t);

//tasklet object
DECLARE_TASKLET(keyboard_tasklet,keyboard_tasklet_fn);
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	//int user_value;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			if(copy_from_user(&num1,(int __user *)arg,sizeof(int)))
			{					
				return -EFAULT;
			}
			pr_info("Kernel: received %d form user\n",num1);
			
			tasklet_schedule(&keyboard_tasklet);
			flag =1;
			break;
		default:
			return -EFAULT;
	}
	return 0;
}
//tasklet bottom half
static void keyboard_tasklet_fn(struct tasklet_struct *t)
{
//	wait_event_interruptible(wq , flag != 0);

//	flag = 0;	
	res = num1 + num2;
	//pr_info("TASKLET: BOTTOM HALF EXECUTED\n");
}
//IRQ HANDler top half
/*static irqreturn_t keyboard_irq_handler(int irq,void *dev_id)
{

	//pr_info("irq : keyboard interrupt occured\n");

	tasklet_schedule(&keyboard_tasklet);

	return IRQ_HANDLED;
}*/

static irqreturn_t irq_top(int irq,void *dev_id)
{
	return IRQ_WAKE_THREAD;
}
static irqreturn_t irq_thread(int irq,void *dev_id)
{
	wait_event_interruptible(wq , flag != 0);

	//  pr_info("Threaded IRQ handler (can sleep)\n");
	msleep(50);

	printk(KERN_INFO "The res = %d\n",res);
	flag = 0;

	return IRQ_HANDLED;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = basic_ioctl,
};

static int __init tasklet_irq_init(void)
{
	
major = register_chrdev(0, DEVICE_NAME, &fops);		
pr_info("basic_ioctl loaded, major= %d\n",major);
	int ret;
	ret = request_irq(KEYBOARD_IRQ,keyboard_irq_handler,IRQF_SHARED,"kbd_tasklet",(void *)keyboard_irq_handler);
//initializing the wait queue
	init_waitqueue_head(&wq);

	if(ret)
	{
		pr_err("Falied to request IRQ %d\n",KEYBOARD_IRQ);
		return ret;
	}
	return request_threaded_irq(IRQ_NUM,irq_top,irq_thread,IRQF_SHARED,"irq_threaded",(void *)irq_thread);
	//pr_info("tasklet module loaded\n");

}

//module exit
static void __exit tasklet_irq_exit(void)
{
	tasklet_kill(&keyboard_tasklet);
	free_irq(KEYBOARD_IRQ,(void *)keyboard_irq_handler);
	free_irq(IRQ_NUM,(void *)irq_thread);
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("tasklet module unloaded\n");
}

module_init(tasklet_irq_init);
module_exit(tasklet_irq_exit);

