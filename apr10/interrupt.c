#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)
#define DRIVER_NAME "irq_demo_driver"
#define IRQ_NUM 1  /*Example: keyboard irq on x86*/

static int irq_counter=0;
static int major;
static int kernel_value=0;

/*
 * interrupt service routine(ISR)
 * This runs in interrupt context (top half)
 */

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	int user_value;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			/*copy data from user*/
			if(copy_from_user(&user_value, (int __user*)arg,sizeof(int)))
			{
				return -EFAULT;
			}
			pr_info("Kernel:received %d from user\n",user_value);

			/*kernel modidfies data*/

			kernel_value=10;

			/*copy data back to user*/
			if(copy_to_user((int __user*)arg,&kernel_value,sizeof(int)))
			{
				return -EFAULT;
			}
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static irqreturn_t irq_demo_isr(int irq,void *dev_id)
{
	irq_counter++;
	pr_info("%s: Interrupt_received! IRQ=%d count=%d\n",DRIVER_NAME,irq,irq_counter);

	/*
	 * IRQ_HANDLED means
	 * -this interrupt was meant for us
	 */
	return IRQ_HANDLED;
}

static struct file_operations fops={
	.owner=THIS_MODULE,
	.unlocked_ioctl=basic_ioctl,
};

static int __init irq_demo_init(void)
{
	major=register_chrdev(0,DEVICE_NAME,&fops);
	int ret;
	pr_info("%s: Initailizing\n",DRIVER_NAME);

	/*
	 * request_irq arguments:
	 * irq        ->IRQ number
	 * handler    ->ISR function
	 * flags      ->IRQF_shared allows sharing
	 * name       ->visible in /proc/interrupts
	 * dev_id     ->unique identifier(must match free_irq)
	 */
	ret=request_irq(IRQ_NUM ,irq_demo_isr,IRQF_SHARED,DRIVER_NAME,(void *)irq_demo_isr);
	if(ret)
	{
		pr_err("%s: failed to request IRQ %d\n",DRIVER_NAME,IRQ_NUM);
		return ret;
	}

	pr_info("%s: IRQ %d registeres successfully\n",DRIVER_NAME,IRQ_NUM);
	return 0;
}

static void __exit irq_demo_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("%s: Cleaning up\n",DRIVER_NAME);

	/*
	 * free_irq must match
	 *-same IRQ number
	 *-same dev_id pointer
	 */

	free_irq(IRQ_NUM,(void *)irq_demo_isr);

	pr_info("%s: IRQ freed\n",DRIVER_NAME);
}

module_init(irq_demo_init);
module_exit(irq_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pavani");
MODULE_DESCRIPTION("simple linux kernel irq handling example");
