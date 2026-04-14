#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

struct calculator
{
	int num1;
	int num2;
	char op;
};
#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'D'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,struct calculator)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,2,int)

static int major;
static int result;

/*ioctl handler */
static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct calculator data;
        switch(cmd)
        {
        case IOCTL_SET_VALUE:
                /*copy data from user */
                if(copy_from_user(&data, (struct calculator __user *)arg, sizeof(data)))
                        return -EFAULT;

                pr_info("Kernel: received %d %d %c from user\n",data.num1,data.num2,data.op);
		switch(data.op)
		{
			case '+':result=data.num1+data.num2;
				 break;
			case '-':result=data.num1-data.num2;
				 break;
			case '*':result=data.num1*data.num2;
				 break;
			case '/':if(data.num2==0)
					 return -EINVAL;
				 result=data.num1/data.num2;
				 break;
			default:
				 return -EINVAL;
		}
		break;
	case IOCTL_GET_VALUE:
                /*copy data back to user*/
                if(copy_to_user((int __user *)arg, &result, sizeof(int)))
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
