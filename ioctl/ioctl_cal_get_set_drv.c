//baisc_ioctl_drv.c
//Minimal IOCTL example: send an int form user->kernel->modify->return
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "calc_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,struct calc_data*)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,2,int)

struct calc_data
{
	int a;
	int b;
	char op;
};
static int res=0;


static int major;
//static int kernel_value=0;
/*ioctl handler*/
static long calc_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct calc_data data;
	//int user_value;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			//copy data from user
			if(copy_from_user(&data,(struct calc_data*)arg,sizeof(data)))
				return -EFAULT;
			if(data.op=='+')
				res=data.a+data.b;
			else if(data.op=='-')
				res=data.a-data.b;
			else if(data.op=='*')
				res=data.a*data.b;
			else if(data.op=='/')
				res=data.a/data.b;
			else
				res=-1;
			pr_info( "The result is %d\n",res);
			break;
		case IOCTL_GET_VALUE:
			if(copy_to_user((int __user*)arg,&res,sizeof(int)))
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
