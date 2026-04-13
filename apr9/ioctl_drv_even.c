//basic _ioctl_drv.c
//minimal IOCTL exmaple :send an int from user ->kernel ->modify ->return 

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'D'
#define IOCTL_SET_EVEN_VALUE _IOWR(IOCTL_MAGIC,1,int)

static int major;
/*ioctl handler*/

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	int user_value;
	switch(cmd)
	{
		case IOCTL_SET_EVEN_VALUE:
			/*copy data from user*/
			if(copy_from_user(&user_value, (int __user*)arg,sizeof(int)))
			{
				return -EFAULT;
			}
			
			/*kernel modidfies data*/
			if(user_value%2==0)
			{
				user_value=1;
			}
			else
			{
				user_value=0;
			}
			
			/*copy data back to user*/
			if(copy_to_user((int __user*)arg,&user_value,sizeof(int)))
			{
				return -EFAULT;
			}
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

