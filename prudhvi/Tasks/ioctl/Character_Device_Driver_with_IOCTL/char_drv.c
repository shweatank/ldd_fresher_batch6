//minimal IOCTL EXAMPLE:send  an string from user -> kernel ->modify and return

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/string.h>
#define DEVICE_NAME "CHARACTOR DRIVER" 
#define IOCTL_MAGIC 'S'
#define IOCTL_SET_DATA _IOW(IOCTL_MAGIC,1,char*)
#define IOCTL_GET_DATA _IOR(IOCTL_MAGIC,2,char*)
#define IOCTL_GET_WRDATA _IOR(IOCTL_MAGIC,3,int)
#define IOCTL_CLEAR _IO(IOCTL_MAGIC,4)
#define BUF_SIZE 256
static int major;
static int wcount=0;
static char kernel_buffer[BUF_SIZE];
/* ioctl handler*/
int size=0;
static long basic_ioctl(struct file *file ,unsigned int cmd,unsigned long arg)
{

	char temp[BUF_SIZE];
	switch(cmd)
	{
		case IOCTL_SET_DATA:
			if(copy_from_user(&temp,(char __user *)arg,BUF_SIZE))
				return -EFAULT;
			temp[BUF_SIZE-1]='\0';
			for(size=0;size<BUF_SIZE&&temp[size]!='\n';size++);
			temp[size++]='\0';
			pr_info("kernel:recevied %s Data from user\n",temp);
			strscpy(kernel_buffer,temp,BUF_SIZE);
			wcount++;
			break;
		case IOCTL_GET_DATA:
			if(copy_to_user((char __user *)arg,kernel_buffer,size))
				return -EFAULT;
			pr_info("kernel:send %s Data to user\n",kernel_buffer);
			break;
		case IOCTL_CLEAR:
			memset(kernel_buffer,0,BUF_SIZE);
			size=0;
			pr_info("Cleared buffer completed successfully\n");
			break;
		case IOCTL_GET_WRDATA:
			if(copy_to_user((int __user *)arg,&wcount,sizeof(wcount)))
				return -EFAULT;
			pr_info("kernel:send count od write %d Data to user\n",wcount);
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




