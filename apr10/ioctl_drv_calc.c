//basic _ioctl_drv.c
//minimal IOCTL exmaple :send an int from user ->kernel ->modify ->return 

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

struct average
{
	int sum;
	int avg;
	int n;
	int arr[100];
};
#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct average)


static int major;

/*ioctl handler*/

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct average data;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			/*copy data from user*/
			if(copy_from_user(&data, (struct average __user*)arg,sizeof(data)))
			{
				return -EFAULT;
			}
			/*kernel modidfies data*/
			int n=data.n;
			data.sum=0;
			data.avg=0;
			for(int i=0; i<n; i++)
			{
				data.sum+=data.arr[i];
			}
			data.avg=data.sum/n;
			/*copy data back to user*/
			if(copy_to_user((struct average __user*)arg,&data,sizeof(data)))
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

