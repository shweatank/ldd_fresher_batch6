#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

typedef struct calculator{
   int num1;
   int num2;
   char opr;
   int result;
}CAL;

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'D'
#define CALC_IOC_ADD _IOWR(IOCTL_MAGIC, 1,CAL)
#define CALC_IOC_SUB _IOWR(IOCTL_MAGIC, 2,CAL)
#define CALC_IOC_MUL _IOWR(IOCTL_MAGIC, 3,CAL)
#define CALC_IOC_DIV _IOWR(IOCTL_MAGIC, 4,CAL)
#define CALC_IOC_MOD _IOWR(IOCTL_MAGIC, 5,CAL)



static int major;

/*ioctl handler */
static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        CAL C;
	switch(cmd)
	{
	case CALC_IOC_ADD:  if(copy_from_user(&C, (CAL __user *)arg, sizeof(CAL)))
                                     return -EFAULT;
	           	    C.result=C.num1+C.num2;
			      if(copy_to_user((CAL __user *)arg, &C, sizeof(CAL)))
                                  return -EFAULT;
			   break;
        case CALC_IOC_SUB:  if(copy_from_user(&C, (CAL __user *)arg, sizeof(CAL)))
                             return -EFAULT;
                          C.result=C.num1-C.num2;
			    if(copy_to_user((CAL __user *)arg, &C, sizeof(CAL)))
                                  return -EFAULT;
                          break;
        case CALC_IOC_MUL:   if(copy_from_user(&C, (CAL __user *)arg, sizeof(CAL)))
                               return -EFAULT;
			  C.result=C.num1*C.num2;
			    if(copy_to_user((CAL __user *)arg, &C, sizeof(CAL)))
                               return -EFAULT;
			  break;
        case CALC_IOC_DIV:   if(copy_from_user(&C, (CAL __user *)arg, sizeof(CAL)))
                                return -EFAULT;
			    C.result=C.num1/C.num2;
			    if(copy_to_user((CAL __user *)arg, &C, sizeof(CAL)))
                               return -EFAULT;
			  break;
	case CALC_IOC_MOD:   if(copy_from_user(&C, (CAL __user *)arg, sizeof(CAL)))
                               return -EFAULT;
			     C.result=C.num1%C.num2;
			    if(copy_to_user((CAL __user *)arg, &C, sizeof(CAL)))
                              return -EFAULT;
			  break;
	default:
		  return -EINVAL;
		break;
	}
	return 0;
}

static struct file_operations fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = basic_ioctl,
};

static int __init basic_init(void)
{
	major = register_chrdev(0, DEVICE_NAME, &fops);
	pr_info("basic_ioctl loaded, major =%d\n", major);
	return 0;
}

static void __exit basic_exit(void)
{
   unregister_chrdev(major,DEVICE_NAME);
   pr_info("basic_ioctl unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nandini");
MODULE_DESCRIPTION(" IOCTL this module is for educational purpose");

