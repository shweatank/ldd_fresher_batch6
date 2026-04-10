//baisc_ioctl_drv.c
//Minimal IOCTL example: send an int form user->kernel->modify->return
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include<linux/interrupt.h>
#include<linux/kernel.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,int)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,2,int)

#define DRIVER_NAME "irq_demo_driver"
#define IRQ_NUM 1 //Example: Keyboard IRQ on x86

static int irq_counter=0;
static int kernel_buffer;
/*
 * Interrupt service routine(ISR)
 * This runs in interrupt context(top half)
 */
static int major;
/*ioctl handler*/
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			//copy data from user
			if(copy_from_user(&kernel_buffer,(int __user*)arg,sizeof(int)))
				return -EFAULT;
			kernel_buffer+=10;

			break;

		case IOCTL_GET_VALUE:
			//copy data back to user
			if(copy_to_user((int __user*)arg,&kernel_buffer,sizeof(int)))
				return -EFAULT;
			break;
		default :
			return -EINVAL;
	}
	return 0;
}

static irqreturn_t irq_demo_isr(int irq,void *dev_id)
{
        irq_counter++;

        pr_info("%s:Interrupt received! IRQ=%d Count=%d\n",DRIVER_NAME,irq,irq_counter);
        /*
         * IRQ_HANDLED means:
         * This interrupt was meant for us
         */
        return IRQ_HANDLED;
}

static struct file_operations fops={
	.owner=THIS_MODULE,
	.unlocked_ioctl=basic_ioctl,
};

static int __init basic_init(void)
{
	major=register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("basic_ioctl loaded,major=%d\n",major);

	//interrupt
	int ret;
        pr_info("%s: Initializing\n",DRIVER_NAME);

        /*
         * request_irq arguments:
         * irq     -> IRQ number
         * handler -> ISR function
         * flags   -> IRQF_SHARED allows sharing
         * name    -> visible in /proc/interrupts
         * dev_id  -> unique identifier (must match free_irq)
         */
        ret=request_irq(IRQ_NUM,irq_demo_isr,IRQF_SHARED,DRIVER_NAME,(void *)irq_demo_isr);

        if(ret)
        {
                pr_err("%s: Failed to request IRQ %d\n",DRIVER_NAME,IRQ_NUM);
                return ret;
        }
        pr_info("%s: IRQ %d registered successfully\n",DRIVER_NAME,IRQ_NUM);
        return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("baisc_ioctl unloaded\n");

	   pr_info("%s: Cleaning up\n",DRIVER_NAME);

        /*free_irq must match:
         * -same IRQ number
         * -same dev_id pointer
         */
        free_irq(IRQ_NUM,(void *)irq_demo_isr);
        pr_info("%s: IRQ frees\n",DRIVER_NAME);

}
module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
