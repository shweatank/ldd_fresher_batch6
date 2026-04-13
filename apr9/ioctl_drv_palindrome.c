//basic _ioctl_drv.c
//minimal IOCTL exmaple :send an int from user ->kernel ->modify ->return 

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'D'
#define IOCTL_SET_PALINDROME_VALUE _IOWR(IOCTL_MAGIC,1,char[100])


static int major;
/*ioctl handler*/

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	char user_value[100];
	switch(cmd)
	{
		case IOCTL_SET_PALINDROME_VALUE:
			/*copy data from user*/
			if(copy_from_user(user_value,(char __user*)arg,sizeof(user_value)))
			{
				return -EFAULT;
			}
			/*kernel modidfies data*/
			int i=0,j=strlen(user_value)-1,flag=0;
			while(user_value[i]!='\0')
			{
				if(user_value[i]!=user_value[j])
				{
					flag=1;
					break;
				}
				i++;
				j--;
			}
			char buff[10];
			if(flag==1)
			{
				buff[0]='1';
			}
			else
			{
				buff[0]='0';
			}		
			buff[2]='\0';
			/*copy data back to user*/
			if(copy_to_user((char __user*)arg,buff,sizeof(buff)))
			{
				return -EFAULT;
			}
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
	pr_info("Basic_ioctl loaded,major=%d\n",major);
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

