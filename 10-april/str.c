//basic_ioctl_drv.c
//Minimal IOCTL example: send an int from user ->kernel-> modify->return
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#define DEVICE_NAME "basic_ioctl"
struct buff
{
	int size;
	char * str;
};
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct buff)
static int major;
/*ioctl handler*/
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct buff buff;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			/*copy data from use*/
			if(copy_from_user(&buff,(struct buff __user *)arg,sizeof(struct buff)))
				return -EFAULT;
			char * karr;
			karr=kmalloc(sizeof(char)*buff.size,GFP_KERNEL);
			if(copy_from_user(karr,buff.str,sizeof(char)*buff.size))
			{
				kfree(karr);
				return EFAULT;
			}
			karr[buff.size]='\0';
			pr_info("kernel: received %s from user\n",karr);
			kfree(karr);
			/*kernel modifies data*/
			/*copy data back to user*/
			if(copy_to_user((struct buff __user *)arg,&buff,sizeof(struct buff)))
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
