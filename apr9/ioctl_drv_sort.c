//basic _ioctl_drv.c
//minimal IOCTL exmaple :send an int from user ->kernel ->modify ->return 

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

struct sort
{
	int n;
	int arr[100];
};

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct sort)

static int major;

/*ioctl handler*/

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct sort data;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			/*copy data from user*/
			if(copy_from_user(&data, (struct sort __user*)arg,sizeof(data)))
			{
				return -EFAULT;
			}
			
			int n=data.n;
			for(int j=0; j<n-1; j++)
			{
				for(int k=0; k<n-j-1; k++)
				{
					if(data.arr[k]>data.arr[k+1])
					{
						int temp=data.arr[k];
						data.arr[k]=data.arr[k+1];
						data.arr[k+1]=temp;
					}
				}
			}		

			/*kernel modidfies data*/

			/*copy data back to user*/
			if(copy_to_user((struct sort __user*)arg,&data,sizeof(data)))
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

