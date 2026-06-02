#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/string.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC, 1,int)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC, 1, char)
static int major;
static int kernel_value = 0;

static char str[10];
static int str_size;
/*ioctl handler */
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	int user_num;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:

			if(copy_from_user(&user_num,(int __user * ) arg, sizeof(int)))
				return -EFAULT;

			pr_info("Kernel: received %d from user\n",user_num);

			//kernel_value = user_value +10;//
			if(user_num%2==0)
			{
				strcpy(str,"even");
			}
			else
				strcpy(str,"odd");
			
			str_size=strlen(str);
			break;

		case IOCTL_GET_VALUE:

			if(copy_to_user((char __user *)arg, str, strlen(str)+1))
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
