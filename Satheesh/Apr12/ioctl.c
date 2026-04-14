#include<linux/init.h>
#include<linux/uaccess.h>
#include<linux/module.h>
#include<linux/uaccess.h>
#define THIS_FILE "ioctl_practice"
static int n;
#define MAGIC_NUMBER 'Z'
#define IOR _IOR(MAGIC_NUMBER,1,int)
#define IOW _IOW(MAGIC_NUMBER,2,int)
static int major;
static long my_ioctl(struct file*file,unsigned	int cmd,unsigned long arg)
{
	switch (cmd){
		case IOR:
			if(copy_to_user((int __user*)arg,&n,sizeof(int)))
					return -EFAULT;
			break;
		case IOW:
			if(copy_from_user(&n,(int __user*)arg,sizeof(int)))
					return -EFAULT;
			n+=10;
			break;
		default:
			return -EFAULT;
	}
	return 0;	

}
static struct file_operations fops={.owner=THIS_MODULE,.unlocked_ioctl=my_ioctl};
static int __init ioctl_init(void)
{
	major=register_chrdev(0,THIS_FILE,&fops);
	if(major<0)
	{
		printk(KERN_ERR"Failed to create chr driver\n");
		return major;
	}
	printk(KERN_INFO"Major=%d\n",major);
return 0;
}
static void __exit ioctl_exit(void)
{
	unregister_chrdev(major,THIS_FILE);
	printk(KERN_INFO"Unloaded kernel module\n");
}
module_init(ioctl_init);
module_exit(ioctl_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SATHEESH");
MODULE_DESCRIPTION("Revising IOCTL");
