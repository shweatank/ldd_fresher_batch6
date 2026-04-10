#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/string.h>

#define DEVICE_NAME "config_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_CONFIG _IOW(IOCTL_MAGIC, 1,struct Operation)
#define IOCTL_GET_CONFIG _IOR(IOCTL_MAGIC, 2,struct Operation)
#define IOCTL_RESET      _IO(IOCTL_MAGIC, 3)

static int major;

/*ioctl handler */
struct Operation
{
	int node;
	int speed;
	char name[32];
};
static struct Operation kernel_value;
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	//struct Operation user_value;

	switch(cmd)
	{
		case IOCTL_SET_CONFIG:

			if(copy_from_user(&kernel_value,(struct Operation __user * ) arg, sizeof(struct Operation)))
				return -EFAULT;

			//pr_info("Kernel: received %d from user\n",user_value);

			//kernel_value = user_value;
			break;

		case IOCTL_GET_CONFIG:

			if(copy_to_user((int __user *)arg, &kernel_value, sizeof(struct Operation)))
				return -EFAULT;
			break;

		case IOCTL_RESET:

			memset(&kernel_value,0,sizeof(struct Operation));
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
