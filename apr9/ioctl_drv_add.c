//basic _ioctl_drv.c
//minimal IOCTL exmaple :send an int from user ->kernel ->modify ->return 

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
struct cal
{
	int n1;
	int n2;
	int res;
	char op;
};

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct cal)


static int major;

/*ioctl handler*/

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct cal data;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			/*copy data from user*/
			if(copy_from_user(&data, (struct cal __user*)arg,sizeof(data)))
			{
				return -EFAULT;
			}
			/*kernel modidfies data*/
			switch(data.op)
                        {
                                case '+':
                                        data.res=data.n1+data.n2;
                                        break;
                                case '-':
                                        data.res=data.n1-data.n2;
                                        break;
                                case '*':
                                        data.res=data.n1*data.n2;
                                        break;
                                case '/':
                                        data.res=data.n1/data.n2;
                                        break;
                        }
			/*copy data back to user*/
			if(copy_to_user((struct cal __user*)arg,&data,sizeof(data)))
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

