//basic _ioctl_drv.c
//minimal IOCTL exmaple :send an int from user ->kernel ->modify ->return 

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

struct buffer
{
	int n;
	char *str;
};
#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct buffer)


static int major;
/*ioctl handler*/

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct buffer data;
	char *user_value;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			/*copy data from user*/
			if(copy_from_user(&data,(struct buffer __user*)arg,sizeof(data)))
			{
				return -EFAULT;
			}
			/*kernel modifies data*/
			user_value=kmalloc(data.n,GFP_KERNEL);
			if(!user_value)
			{
				return -ENOMEM;
			}
			if(copy_from_user(user_value,data.str,data.n))
			{
				kfree(user_value);
				return -EFAULT;
			}
			/*copy data back to user*/
			if(copy_to_user(data.str,user_value,data.n))
			{
				kfree(user_value);
				return -EFAULT;
			}
			kfree(user_value);
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static struct file_operations fops={
	.owner=THIS_MODULE,
	.unlocked_ioctl=basic_ioctl,
};

static int __init basic_init(void)
{
	major=register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("Basic_ioctl loaded,major=%d\n",major);
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

