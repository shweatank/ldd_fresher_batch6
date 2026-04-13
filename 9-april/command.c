//basic_ioctl_drv.c
//Minimal IOCTL example: send an int from user ->kernel-> modify->return
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#define DEVICE_NAME "basic_ioctl"

#define IOC_MAGIC 'C'
struct calc
{
	int a;
	int b;
	long result;
};
#define CALC_IOC_ADD _IOWR(IOC_MAGIC,1,struct calc)
#define CALC_IOC_SUB _IOWR(IOC_MAGIC,2,struct calc)
#define CALC_IOC_MUL _IOWR(IOC_MAGIC,3,struct calc)
#define CALC_IOC_DIV _IOWR(IOC_MAGIC,4,struct calc)
#define CALC_IOC_MOD _IOWR(IOC_MAGIC,5,struct calc)
static int major;
/*ioctl handler*/
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct calc data;
	switch(cmd)
	{
		case CALC_IOC_ADD:
			/*copy data from use*/
			if(copy_from_user(&data,(struct data __user *)arg,sizeof(data)))
				return -EFAULT;
			pr_info("kernel: received %d %d from user\n",data.a,data.b);
			/*kernel modifies data*/
			data.result=data.a+data.b;
			/*copy data back to user*/
			if(copy_to_user((struct data __user *)arg,&data,sizeof(data)))
				return -EFAULT;
			break;
		case CALC_IOC_SUB:if(copy_from_user(&data,(struct data __user *)arg,sizeof(data)))
                                return -EFAULT;
                        pr_info("kernel: received %d %d from user\n",data.a,data.b);
                        /*kernel modifies data*/
			data.result=data.a-data.b;
                        /*copy data back to user*/
                        if(copy_to_user((struct data __user *)arg,&data,sizeof(data)))
                                return -EFAULT;
                        break;
		case CALC_IOC_MUL:if(copy_from_user(&data,(struct data __user *)arg,sizeof(data)))
                                return -EFAULT;
                        pr_info("kernel: received %d %d from user\n",data.a,data.b);
                        /*kernel modifies data*/
                        data.result=data.a*data.b;
		       	/*copy data back to user*/
                        if(copy_to_user((struct data __user *)arg,&data,sizeof(data)))
                                return -EFAULT;
                        break;
		case CALC_IOC_DIV:if(copy_from_user(&data,(struct data __user *)arg,sizeof(data)))
                                return -EFAULT;
                        pr_info("kernel: received %d %d from user\n",data.a,data.b);
                        /*kernel modifies data*/
                        data.result=data.a/data.b;
                        /*copy data back to user*/
                        if(copy_to_user((struct data __user *)arg,&data,sizeof(data)))
                                return -EFAULT;
                        break;
		case CALC_IOC_MOD:if(copy_from_user(&data,(struct data __user *)arg,sizeof(data)))
                                return -EFAULT;
                        pr_info("kernel: received %d %d from user\n",data.a,data.b);
                        /*kernel modifies data*/
                        data.result=data.a%data.b;
                        /*copy data back to user*/
                        if(copy_to_user((struct data __user *)arg,&data,sizeof(data)))
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
