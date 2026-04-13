//basic _ioctl_drv.c
//minimal IOCTL exmaple :send an int from user ->kernel ->modify ->return 

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
struct rev
{
	char str[10];
};

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_REV_VALUE _IOWR(IOCTL_MAGIC,1,struct rev)


static int major;

/*ioctl handler*/

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct rev data;
	switch(cmd)
	{
		case IOCTL_SET_REV_VALUE:
			/*copy data from user*/
			if(copy_from_user(&data, (struct rev __user*)arg,sizeof(data)))
			{
				return -EFAULT;
			}
			/*kernel modidfies data*/
			int len=strlen(data.str);
			int start=0,end=len-1;
			while(start<end)
			{
				char temp=data.str[start];
				data.str[start]=data.str[end];
				data.str[end]=temp;
				start++;
				end--;
			}
			/*copy data back to user*/
			if(copy_to_user((struct rev __user*)arg,&data,sizeof(data)))
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


