//baisc_ioctl_drv.c
//Minimal IOCTL example: send an int form user->kernel->modify->return
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,char[100])

static int major;
static char str[100];
/*ioctl handler*/
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	int user_value;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			//copy data from user
			if(copy_from_user(str,(char __user*)arg,sizeof(str)))
				return -EFAULT;
			pr_info("kernel: received %d from user\n",user_value);

			//operation
			for(int i=0;str[i]!='\0';i++)
			{
				if((str[i]>=65 && str[i]<=90) || (str[i]>=97&&str[i]<=122))
						str[i]^=32;
			}
			pr_info("The string is %s\n",str);

			//copy data back to user
			if(copy_to_user((char __user*)arg,str,sizeof(str)))
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
