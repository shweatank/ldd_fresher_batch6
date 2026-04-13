//basic _ioctl_drv.c
//minimal IOCTL exmaple :send an int from user ->kernel ->modify ->return 

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_CASE_VALUE _IOWR(IOCTL_MAGIC,1,char[100])


static int major;
/*ioctl handler*/

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	char user_value[100];
	switch(cmd)
	{
		case IOCTL_SET_CASE_VALUE:
			/*copy data from user*/
			if(copy_from_user(user_value,(char __user*)arg,sizeof(user_value)))
			{
				return -EFAULT;
			}
			/*kernel modidfies data*/
			int i=0;
			while(user_value[i]!='\0')
			{
				if(user_value[i]>='a' && user_value[i]<='z')
				{
					user_value[i]=user_value[i]-32;
				}
				else if(user_value[i]>='A' && user_value[i]<='Z')
				{
					user_value[i]=user_value[i]+32;
				}
				i++;
			}		
			/*copy data back to user*/
			if(copy_to_user((char __user*)arg,user_value,strlen(user_value)+1))
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
