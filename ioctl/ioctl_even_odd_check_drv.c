//baisc_ioctl_drv.c
//Minimal IOCTL example: send an int form user->kernel->modify->return
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "calc_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,int)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,2,char[10])

char kernel_buffer[10];


static int major;
//static int kernel_value=0;
/*ioctl handler*/
static long calc_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	int data;
	//int user_value;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			//copy data from user
			if(copy_from_user(&data,(int __user*)arg,sizeof(data)))
				return -EFAULT;
			if(data&1)
				strcpy(kernel_buffer,"ODD");
			else
				strcpy(kernel_buffer,"EVEN");
			break;

		case IOCTL_GET_VALUE:
			if(copy_to_user((char __user*)arg,&kernel_buffer,sizeof(kernel_buffer)))
				return -EFAULT;
			break;
		default :
			return -EINVAL;
	}
	return 0;
}
static struct file_operations fops={
	.owner=THIS_MODULE,
	.unlocked_ioctl=calc_ioctl,
};

static int __init basic_init(void)
{
	major=register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("calc_ioctl loaded,major=%d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("calc_ioctl unloaded\n");
}
module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
