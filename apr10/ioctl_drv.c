//basic _ioctl_drv.c
//minimal IOCTL exmaple :send an int from user ->kernel ->modify ->return 

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
struct device
{
	int mode;
	int speed;
	char name[100];
};

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'D'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,struct device)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,2,struct device)
#define IOCTL_RESET_VALUE _IOWR(IOCTL_MAGIC,3,struct device)


static int major;
/*ioctl handler*/

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct device data;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			/*copy data from user*/
			if(copy_from_user(&data,(struct device __user*)arg,sizeof(data)))
			{
				return -EFAULT;
			}
			/*kernel modidfies data*/
			printk("kernel:%d %d %s\n",data.mode,data.speed,data.name);
			/*copy data back to user*/
			if(copy_to_user((struct device __user*)arg,&data,sizeof(data)))
			{
				return -EFAULT;
			}
			break;
		case IOCTL_GET_VALUE:
			if(copy_from_user(&data,(struct device __user*)arg,sizeof(data)))
                        {
                                return -EFAULT;
                        }
                        /*kernel modidfies data*/
                        /*copy data back to user*/
                        if(copy_to_user((struct device __user*)arg,&data,sizeof(data)))
                        {
                                return -EFAULT;
                        }
                        break;
		case IOCTL_RESET_VALUE:
			if(copy_from_user(&data,(struct device __user*)arg,sizeof(data)))
                        {
                                return -EFAULT;
                        }
                        /*kernel modidfies data*/
                        data.mode=0;
			data.speed=0;
			strcpy(data.name,"default");
                        /*copy data back to user*/
                        if(copy_to_user((struct device  __user*)arg,&data,sizeof(data)))
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

