#include<linux/uaccess.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/slab.h>
#include<linux/fs.h>
static int major;
static struct conf
{
	int mode,w_count;
	char buff[256];
};
static struct conf settings;
#define MAJOR_NUMBER 'S'
#define DEVICE_NAME "conf_settings"
#define SET_MODE _IOW(MAJOR_NUMBER,1,int)
#define GET_MODE _IOR(MAJOR_NUMBER,2,int)
#define CLEAR_BUFFER _IOW(MAJOR_NUMBER,3,char)
#define GET_WRITE_COUNT _IOR(MAJOR_NUMBER,4,int)
#define SET_ALL _IOW(MAJOR_NUMBER,5,struct conf)
#define GET_ALL _IOR(MAJOR_NUMBER,6,struct conf)
static long my_ioctl(struct file*file,unsigned cmd,unsigned long arg)
{
	switch (cmd)
	{
		case SET_MODE:
			if(copy_from_user(&settings.mode,(int __user*)arg,sizeof(int)))
				return -EFAULT;
			settings.w_count++;
			break;
		case GET_MODE:
			if(copy_to_user((int __user*)arg,&settings.mode,sizeof(int)))
				return -EFAULT;
			break;
		case CLEAR_BUFFER:
			settings.buff[0]='\0';
			settings.w_count++;
			break;
		case GET_WRITE_COUNT:
			if(copy_to_user((int  __user*)arg,&settings.w_count,sizeof(int)))
				return -EFAULT;
			break;


		default:
			pr_info("Enter a valid command\n");
			return -EFAULT;
			break;
	}
	return 0;
}
static int my_open(struct inode *inode,struct file *file)
{
	printk("Char driver opened\n");
return 0;
}
static int my_release(struct inode*inode,struct file*file)
{
	pr_info("Chr driver closed\n");
	return 0;
}
static ssize_t my_write(struct file* file, const char __user * arg,size_t count,loff_t *offset)
{
	if(copy_from_user(&settings,(struct conf __user*)arg,sizeof(struct conf)))
		return -EFAULT;
	return 0;
}
static ssize_t my_read(struct file*file, char __user *arg,size_t count,loff_t *offset)
{
	if(copy_to_user((struct conf __user *)arg,&settings,sizeof(struct conf)))
			return -EFAULT;
	return 0;
}
static struct file_operations fops={.owner=THIS_MODULE,.unlocked_ioctl=my_ioctl,.open=my_open,.release=my_release,.read=my_read,.write=my_write};
static int __init my_init(void)
{
	major=register_chrdev(0,DEVICE_NAME,&fops);
	if(major<0)
	{
		printk(KERN_ERR"Failed to create chr driver\n");
		return -EFAULT;
	}
	printk("Driver loaded major=%d\n",major);
return 0;
}
static void __exit my_exit(void){
	unregister_chrdev(major,DEVICE_NAME);
	printk(KERN_INFO"Driver unloaded\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SATHEESH");
MODULE_DESCRIPTION("Conf setting task");
