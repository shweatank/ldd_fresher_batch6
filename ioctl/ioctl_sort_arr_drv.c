//baisc_ioctl_drv.c
//Minimal IOCTL example: send an int form user->kernel->modify->return
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct op_data)

static int major;
struct op_data
{
	int arr[100];
	int size;
};

/*ioctl handler*/
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct op_data data;
	int temp;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			//copy data from user
			if(copy_from_user(&data,(struct op_data*)arg,sizeof(data)))
				return -EFAULT;
			for(int i=0;i<data.size;i++)
			{
				for(int j=i+1;j<data.size;j++)
				{
					if(data.arr[i]>data.arr[j])
					{
						temp=data.arr[i];
						data.arr[i]=data.arr[j];
						data.arr[j]=temp;
					}
				}
			}
			for(int i=0;i<data.size;i++)
			{
				pr_info("%d ",data.arr[i]);
			}


			//copy data back to user
			if(copy_to_user((struct op_data*)arg,&data,sizeof(data)))
				return -EFAULT;
			break;
		default :
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
	pr_info("basic_ioctl loaded,major=%d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("baisc_ioctl unloaded\n");
}
module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
