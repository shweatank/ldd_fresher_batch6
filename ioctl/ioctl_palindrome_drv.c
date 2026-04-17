//baisc_ioctl_drv.c
//Minimal IOCTL example: send an int form user->kernel->modify->return
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,char[100])

static int major;
static char kernel_buffer[100];

/*ioctl handler*/
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			//copy data from user
			if(copy_from_user(kernel_buffer,(char __user*)arg,sizeof(kernel_buffer)))
				return -EFAULT;
			pr_info("kernel string %s\n",kernel_buffer);
			int left=0,right=strlen(kernel_buffer)-1;
			while(left<right)
			{
				if(kernel_buffer[left]!=kernel_buffer[right])
					break;
				left++;
				right--;
			}
			if(left<right)
				strcpy(kernel_buffer,"Not palindrome");
			else
				strcpy(kernel_buffer,"Palindrome");

			//copy data back to user
			if(copy_to_user((char __user*)arg,kernel_buffer,sizeof(kernel_buffer)))
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
