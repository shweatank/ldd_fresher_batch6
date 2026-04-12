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
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,CAL)

static int major;

/*ioctl handler */
static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        CAL C;
	switch(cmd)
	{
	case IOCTL_SET_VALUE:
		/*copy data from user */
		if(copy_from_user(&C, (CAL __user *)arg, sizeof(CAL)))
			return -EFAULT;

		pr_info("Kernel: received %d from user\n", C.num1);
		pr_info("Kernel: received %d from user\n", C.num2);
		switch(C.opr)
		{
		 case '+':
		          C.result=C.num1+C.num2;
			  break;
		 case '-':
                          C.result=C.num1-C.num2;
                          break;
	         case '*':
			  C.result=C.num1*C.num2;
			  break;
		 case '/': 
			  C.result=C.num1/C.num2;
			  break;
		 default:
			  return -EINVAL;

		}

		/*copy data back to user*/
		if(copy_to_user((CAL __user *)arg, &C, sizeof(CAL)))
			return -EFAULT;
		break;
	default:
		return -EINVAL;
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

