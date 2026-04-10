#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>

#define DEVICE_NAME "basic ioctl"
#define IOCTL_MAGIC 'C'
//#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)
static int major;
#define CALC_IOC_MAGIC 'C'
static int result=-1;
//static int kernel_value=0;
struct calc
{
	int a,b;
};
#define CALC_IOC_WADD _IOW(CALC_IOC_MAGIC,1,struct calc)
#define CALC_IOC_RADD _IOR(CALC_IOC_MAGIC,2,int)

#define CALC_IOC_WSUB _IOW(CALC_IOC_MAGIC,3,struct calc)
#define CALC_IOC_RSUB _IOR(CALC_IOC_MAGIC,4,int)

#define CALC_IOC_WMUL _IOW(CALC_IOC_MAGIC,5,struct calc)
#define CALC_IOC_RMUL _IOR(CALC_IOC_MAGIC,6,int)

#define CALC_IOC_WDIV _IOW(CALC_IOC_MAGIC,7,struct calc)
#define CALC_IOC_RDIV _IOR(CALC_IOC_MAGIC,8,int)
static long basic_ioctl(struct file*file , unsigned int cmd,unsigned long arg)
{
	struct calc user_value;
	switch(cmd)
	{
		case CALC_IOC_WADD:
			if(copy_from_user(&user_value,(struct calc __user*)arg,sizeof(struct calc)))
				return -EFAULT;
		
			pr_info("Kernel: Received %d %d from user\n",user_value.a,user_value.b);
		// Kernel Modifies the data
		//kernel_value=user_value+10;
		//Copy back to user
			result=user_value.a+user_value.b;
			break;
		case CALC_IOC_RADD: 
			if(copy_to_user((int __user*)arg,&result,sizeof(int)))
				return -EFAULT;
			break;
		case CALC_IOC_WSUB:
			if(copy_from_user(&user_value,(struct calc __user*)arg,sizeof(struct calc)))
				return -EFAULT;
			
			pr_info("Kernel: Received %d %d from user\n",user_value.a,user_value.b);
			result=user_value.a-user_value.b;
			break;
		case CALC_IOC_RSUB: 
			if(copy_to_user((int __user*)arg,&result,sizeof(int)))
				return -EFAULT;
			break;
		case CALC_IOC_WMUL:
			if(copy_from_user(&user_value,(struct calc __user*)arg,sizeof(struct calc)))
				return -EFAULT;
			
			pr_info("Kernel: Received %d %d from user\n",user_value.a,user_value.b);
			result=user_value.a*user_value.b;
			break;
		case CALC_IOC_RMUL: 
			if(copy_to_user((int __user*)arg,&result,sizeof(int)))
				return -EFAULT;
			break;
		case CALC_IOC_WDIV:
			if(copy_from_user(&user_value,(struct calc __user*)arg,sizeof(struct calc)))
				return -EFAULT;
			
			pr_info("Kernel: Received %d %d from user\n",user_value.a,user_value.b);
			if(user_value.b==0)
				return -EFAULT;
			result=user_value.a/user_value.b;
			break;
		case CALC_IOC_RDIV: 
			if(copy_to_user((int __user*)arg,&result,sizeof(int)))
				return -EFAULT;
			break;

			default:
			pr_info("kernel:Enter valid oprion\n");
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
//MODULE_DESCRIPTION("Basic IOCTL");







