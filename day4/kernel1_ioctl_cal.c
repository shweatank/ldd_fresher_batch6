#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

struct calculator
{
	int num1;
	int num2;
	char op;
	int result;
};
#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'D'
#define CAL_IOCTL_ADD _IOWR(IOCTL_MAGIC,1,struct calculator)
#define CAL_IOCTL_SUB _IOWR(IOCTL_MAGIC,2,struct calculator)
#define CAL_IOCTL_MUL _IOWR(IOCTL_MAGIC,3,struct calculator)
#define CAL_IOCTL_DIV _IOWR(IOCTL_MAGIC,4,struct calculator)
#define CAL_IOCTL_MOD _IOWR(IOCTL_MAGIC,5,struct calculator)

static int major;

/*ioctl handler */
static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct calculator data;
        switch(cmd)
        {
        case CAL_IOCTL_ADD:
                /*copy data from user */
                if(copy_from_user(&data, (struct calculator __user *)arg, sizeof(data)))
                        return -EFAULT;
		data.result=data.num1+data.num2;
                /*copy data back to user*/
                if(copy_to_user((struct calculator __user *)arg, &data, sizeof(data)))
                        return -EFAULT;
                break;
	case CAL_IOCTL_SUB:
                /*copy data from user */
                if(copy_from_user(&data, (struct calculator __user *)arg, sizeof(data)))
                        return -EFAULT;
                data.result=data.num1-data.num2;
                /*copy data back to user*/
                if(copy_to_user((struct calculator __user *)arg, &data, sizeof(data)))
                        return -EFAULT;
                break;
	case CAL_IOCTL_MUL:
                /*copy data from user */
                if(copy_from_user(&data, (struct calculator __user *)arg, sizeof(data)))
                        return -EFAULT;
                data.result=data.num1*data.num2;
                /*copy data back to user*/
                if(copy_to_user((struct calculator __user *)arg, &data, sizeof(data)))
                        return -EFAULT;
                break;
	case CAL_IOCTL_DIV:
                /*copy data from user */
                if(copy_from_user(&data, (struct calculator __user *)arg, sizeof(data)))
                        return -EFAULT;
                data.result=data.num1/data.num2;
                /*copy data back to user*/
                if(copy_to_user((struct calculator __user *)arg, &data, sizeof(data)))
                        return -EFAULT;
                break;
	case CAL_IOCTL_MOD:
                /*copy data from user */
                if(copy_from_user(&data, (struct calculator __user *)arg, sizeof(data)))
                        return -EFAULT;
                data.result=data.num1%data.num2;
                /*copy data back to user*/
                if(copy_to_user((struct calculator __user *)arg, &data, sizeof(data)))
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
