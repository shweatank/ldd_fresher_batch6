//basic_ioctl_drv.c
//Minimal IOCTL example: send an int from user ->kernel-> modify->return
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#define DEVICE_NAME "basic_ioctl"
struct data
{
	int size;
	int * arr;
};
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct data)
static int major;
static int *karr;
/*ioctl handler*/
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct data data;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			/*copy data from use*/
			if(copy_from_user(&data,(struct data __user *)arg,sizeof(struct data)))
				return -EFAULT;
			//pr_info("kernel: received %d from user\n",user_value);
			karr=kmalloc(sizeof(int)*data.size,GFP_KERNEL);
			if(copy_from_user(karr,data.arr,sizeof(int)*data.size))
			{
				kfree(karr);
                                return -EFAULT;
			}
			/*kernel modifies data*/
			int i,j;
			for(i=0;i<data.size-1;i++)
			{
				for(j=0;j<data.size-i-1;j++)
				{
					if(karr[j]>karr[j+1])
					{
						int temp=karr[j];
						karr[j]=karr[j+1];
						karr[j+1]=temp;
					}
				}
			}
			if(copy_to_user(data.arr,karr,sizeof(int)*data.size))
                        {
				kfree(karr);
                                return -EFAULT;
                        }
			kfree(karr);
			/*copy data back to user*/
			if(copy_to_user((struct data __user *)arg,&data,sizeof(struct data)))
				return -EFAULT;
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
	pr_info("basic_ioctl loaded, major=%d\n",major);
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
