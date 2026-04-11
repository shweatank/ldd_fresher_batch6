#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>

#define DEVICE_NAME "basic ioctl"
//#define IOCTL_MAGIC 'B'
#define CALC_IOC_MAGIC 'C'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)

struct calc
{
	int a,b,result;
};
#define CALC_IOC_ADD _IOWR(CALC_IOC_MAGIC,1,struct calc)
#define CALC_IOC_SUB _IOWR(CALC_IOC_MAGIC,2,struct calc)
#define CALC_IOC_MUL _IOWR(CALC_IOC_MAGIC,3,struct calc)
#define CALC_IOC_DIV _IOWR(CALC_IOC_MAGIC,4,struct calc)

static struct calc cal;
static int major;
static int kernel_value=0;

static long basic_ioctl(struct file*file , unsigned int cmd,unsigned long arg)
{
	struct calc user_value;
	switch(cmd)
	{
	/*	//case IOCTL_SET_VALUE:
		if(copy_from_user(&user_value,(int __user*)arg,sizeof(int)))
		return -EFAULT;
		
		pr_info("Kernel: Received %d from user\n",user_value);
		// Kernel Modifies the data
		//kernel_value=user_value+10;
		//Copy back to user 
		//if(copy_to_user((int __user*)arg,&kernel_value,sizeof(int)))
		//	return -EFAULT;
	break;
		default:
			return -EFAULT;*/
	case CALC_IOC_ADD:
		if(copy_from_user(&user_value,(struct calc __user*)arg,sizeof(struct calc)))
		return -EFAULT;
		cal.a=user_value.a;
		cal.b=user_value.b;
		cal.result=cal.a+cal.b;
		if(copy_to_user((struct calc __user*)arg,&cal,sizeof(struct calc)))
			return -EFAULT;
		break;
			
	case CALC_IOC_SUB:
		if(copy_from_user(&user_value,(struct calc __user*)arg,sizeof(struct calc)))
		return -EFAULT;
		cal.a=user_value.a;
		cal.b=user_value.b;
		cal.result=cal.a-cal.b;
		if(copy_to_user((struct calc __user*)arg,&cal,sizeof(struct calc)))
			return -EFAULT;
		break;
	case CALC_IOC_MUL:
		if(copy_from_user(&user_value,(struct calc __user*)arg,sizeof(struct calc)))
		return -EFAULT;
		cal.a=user_value.a;
		cal.b=user_value.b;
		cal.result=cal.a*cal.b;
		if(copy_to_user((struct calc __user*)arg,&cal,sizeof(struct calc)))
			return -EFAULT;
		break;
	case CALC_IOC_DIV:
		if(copy_from_user(&user_value,(struct calc __user*)arg,sizeof(struct calc)))
		return -EFAULT;
		cal.a=user_value.a;
		cal.b=user_value.b;
		if(cal.b==0)
			return -EFAULT;
		cal.result=cal.a/cal.b;
		if(copy_to_user((struct calc __user*)arg,&cal,sizeof(struct calc)))
			return -EFAULT;
		break;
	default:
		return -EFAULT;	
	}
return 0;
}
static struct file_operations fops={
					.owner=THIS_MODULE,
					.unlocked_ioctl=basic_ioctl
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
MODULE_AUTHOR("Satheesh Kumar");
MODULE_DESCRIPTION("Basic IOCTL");







