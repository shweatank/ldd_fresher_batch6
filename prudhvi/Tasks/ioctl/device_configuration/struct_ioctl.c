//minimal IOCTL EXAMPLE:send  an int from user -> kernel ->modify and return

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/string.h>
struct config
{
int mode;
int speed;
char name[32];
};
static struct config ker_stu = {
.mode=0,
.speed=100,
.name="default"
};
#define DEVICE_NAME "structure_ioctl" 
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_CONFIG _IOW(IOCTL_MAGIC,1,struct config)
#define IOCTL_GET_CONFIG _IOR(IOCTL_MAGIC,2,struct config)
#define IOCTL_RESET _IO(IOCTL_MAGIC,3)

static int major;

/* ioctl handler*/
static long basic_ioctl(struct file *file ,unsigned int cmd,unsigned long arg)
{

	struct config temp;
	switch(cmd)
	{
		case IOCTL_SET_CONFIG:
			if(copy_from_user(&temp,(struct config __user *)arg,sizeof(temp)))
				return -EFAULT;

			pr_info("kernel:recevied strcture from user\n");
			ker_stu=temp;
			break;
		case IOCTL_GET_CONFIG:
			if(copy_to_user((struct config __user *)arg,&ker_stu,sizeof(ker_stu)))
				return -EFAULT;
			pr_info("kernel:send strcture to user\n");
			break;
		case IOCTL_RESET:
			ker_stu.mode=0;
			ker_stu.speed=100;
			strcpy(ker_stu.name,"default");
			pr_info("Default setting completed\n");
			break;
		default:
			return -EINVAL;
	}
	return 0;
}


static struct file_operations fops = {

	.owner	=THIS_MODULE,
	.unlocked_ioctl	=basic_ioctl,
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
pr_info("basic_ioctl unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");




