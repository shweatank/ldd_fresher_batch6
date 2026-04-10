//minimal IOCTL EXAMPLE:send  an int from user -> kernel ->modify and return

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/string.h>
#define DEVICE_NAME "basic_ioctl_drv" 
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)

static int major;
static int kernel_value=0;
struct op
{
int a,b;
char s[10];
};
/* ioctl handler*/
static long basic_ioctl(struct file *file ,unsigned int cmd,unsigned long arg)
{

	struct op u;

	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			if(copy_from_user(&u,(struct op __user *)arg,sizeof(u)))
				return -EFAULT;

			pr_info("kernel:recevied %s %d %d  from user\n",u.s,u.a,u.b);

			if(strcmp(u.s,"add")==0)
				kernel_value=u.a+u.b;
			else if(strcmp(u.s,"sub")==0)
				kernel_value=u.a-u.b;
			else if(strcmp(u.s,"mul")==0)
				kernel_value=u.a*u.b;
			else
				return -EINVAL;
			u.a=kernel_value;
			if(copy_to_user((struct op __user*)arg,&u,sizeof(u)))
				return -EFAULT;
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




