//minimal IOCTL EXAMPLE:send  an int from user -> kernel ->modify and return

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>

#define DEVICE_NAME "basic_ioctl" 
#define IOCTL_MAGIC 'K'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,char*)
#define BUF_SIZE 256
static int major;
static char kernel_buffer[256];

/* ioctl handler*/
static long basic_ioctl(struct file *file ,unsigned int cmd,unsigned long arg)
{

	char user_buffer[BUF_SIZE];
			int i;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			if(copy_from_user(user_buffer,(char __user *)arg,BUF_SIZE))
				return -EFAULT;
			user_buffer[BUF_SIZE - 1] = '\0';
			for(i=0;user_buffer[i]!='\n'&&i<BUF_SIZE;i++);
			user_buffer[i]='\0';
			pr_info("kernel:recevied %s from user\n",user_buffer);

			for(i=0;user_buffer[i];i++)
			{
				if((user_buffer[i]>='a')&&(user_buffer[i]<='z'))
				{
					kernel_buffer[i]=user_buffer[i]-32;
				}
				else if((user_buffer[i]>='A'&&user_buffer[i]<='Z'))
				{
					kernel_buffer[i]=user_buffer[i]+32;
				}
				else
				{

					kernel_buffer[i]=user_buffer[i];
				}
			}
			kernel_buffer[i]='\0';
			pr_info("kernel:sending %s to user\n",kernel_buffer);
			if(copy_to_user((char __user*)arg,kernel_buffer,i+1))
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




