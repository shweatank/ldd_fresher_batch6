#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)
#define DRIVER_NAME "irq_demo_driver"
#define IRQ_NUM 1 /*Example:keyboard IRQ on x86 */

static int irq_counter=0;
static int major;
static int kernel_value = 0;

/*ioctl handler */
static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        int user_value;

        switch(cmd)
        {
        case IOCTL_SET_VALUE:
                /*copy data from user */
                if(copy_from_user(&user_value, (int __user *)arg, sizeof(int)))
                        return -EFAULT;

                pr_info("Kernel: received %d from user\n", user_value);

                /*kernel modifies data*/
                kernel_value=user_value + 10;

                /*copy data back to user*/
                if(copy_to_user((int __user *)arg, &kernel_value, sizeof(int)))
                        return -EFAULT;
                break;
        default:
                return -EINVAL;
        }
        return 0;
}

static struct file_operations fops = {
        .owner = THIS_MODULE,
        .unlocked_ioctl = basic_ioctl,
};

/*
 * Interrupt service routine(ISR)
 * This runs in interrupt context (top half)
 */
static irqreturn_t irq_demo_isr(int irq,void *dev_id)
{
	irq_counter++;
	pr_info("%s :interrupt recieved! IRQ=%d count=%d\n",DRIVER_NAME,irq,irq_counter);

	/*
	 * IRQ_HANDLED means:
	 * -This interrupt was meant for us
	 */
	return IRQ_HANDLED;
}
static int __init irq_demo_init(void)
{
	int ret;
	major = register_chrdev(0, DEVICE_NAME, &fops);
        pr_info("basic_ioctl loaded, major =%d\n", major);
	pr_info("%s:Initializing\n",DRIVER_NAME);
	/*
	 * request_irq arguments
	 * irq ->IRQ number
	 * handler ->ISR function
	 * flags ->IRQF_SHARED allows sharing
	 * name ->visible in /proc/interrupts
	 * dev_id ->unique identifier (must watch free_irq)
	 */
	ret=request_irq(IRQ_NUM,irq_demo_isr,IRQF_SHARED,DRIVER_NAME,(void *)irq_demo_isr);
	if(ret)
	{
		pr_err("%s:Failed to request IRQ %d\n",DRIVER_NAME,IRQ_NUM);
		return ret;
	}
	pr_info("%s:IRQ %d registered successfully\n",DRIVER_NAME,IRQ_NUM);
	return 0;
}
static void __exit irq_demo_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
        pr_info("basic_ioctl unloaded\n");
	pr_info("%s:Cleaning up\n",DRIVER_NAME);
	/*
	 * free_irq must match:
	 * -Same IRQ number
	 * -same dev_id pointer
	 */
	free_irq(IRQ_NUM,(void *)irq_demo_isr);
	pr_info("%s:IRQ Freed\n",DRIVER_NAME);
}
module_init(irq_demo_init);
module_exit(irq_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sarika");
MODULE_DESCRIPTION("Simple Linux kernel IRQ Handler Example");
