//basic_ioctl_drv.c
//Minimal IOCTL example: send an int from user ->kernel-> modify->return
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,char *)
static int major;
static char kernel_value[100];
/*ioctl handler*/
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	char user_value[100];
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			/*copy data from use*/
			if(copy_from_user(user_value,(char* __user *)arg,sizeof(user_value)))
				return -EFAULT;
			pr_info("kernel: received %s from user\n",user_value);
			/*kernel modifies data*/
			int i,j,flag=0;
			i=0,j=strlen(user_value)-1;
			while(i<j)
			{
				if(user_value[i]!=user_value[j])
				{
					flag=1;
					strcpy(kernel_value,"not a palindrome");
					break;
				}
				i++;
				j--;
			}
			if(!flag)
				strcpy(kernel_value,"palindrome");
			/*copy data back to user*/
			if(copy_to_user((char * __user *)arg,kernel_value,sizeof(kernel_value)))
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

