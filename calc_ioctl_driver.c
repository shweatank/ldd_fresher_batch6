#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/string.h>
#include<linux/kernel.h>


#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC, 1,struct Operation)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC, 1, int)
static int major;
static int kernel_value = 0;

/*ioctl handler */
struct Operation
{
	int num1;
	int num2;
	char opp[4];
};
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct Operation user_value;

	switch(cmd)
	{
		case IOCTL_SET_VALUE:

			if(copy_from_user(&user_value,(struct Operation __user * ) arg, sizeof(struct Operation)))
				return -EFAULT;

			//pr_info("Kernel: received %d from user\n",user_value);

			//kernel_value = user_value +10;//
			if(strcmp(user_value.opp,"add")==0)
			{
				kernel_value=user_value.num1+user_value.num2;
			}
			else if(strcmp(user_value.opp,"sub")==0)
			{
				kernel_value=user_value.num1-user_value.num2;
			}
			else if(strcmp(user_value.opp,"mul")==0)
			{
				kernel_value=user_value.num1*user_value.num2;
			}
			else if(strcmp(user_value.opp,"div")==0)
			{
				kernel_value=user_value.num1/user_value.num2;
			}
			else
			{
				pr_info("Enter correct Operation !");
				return 0;
			}

			break;

		case IOCTL_GET_VALUE:

			if(copy_to_user((int __user *)arg, &kernel_value, sizeof(int)))
				return -EFAULT;
			break;

		default:
			return -EINVAL;
	}
	return 0;
}

static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = basic_ioctl,
};

static int __init basic_init(void)
{
	major = register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("basic ioctl loaded, major=%d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major, DEVICE_NAME);
	pr_info("basic_ioctl unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
