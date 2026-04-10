#include<linux/init.h>
#include<linux/uaccess.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/module.h>
#define MAGIC_NUMBER 'B'
#define DEVICE_NAME "case_convert"
#define SIZE 100
#define CASE_CONVERT _IOWR(MAGIC_NUMBER,1,char *)
static char *str;
static int major;

static void transform(char *ptr)
{
	for(int i=0;ptr[i];i++)
	{
		if(ptr[i] >='a' && ptr[i]<='z')
		{
			ptr[i]=ptr[i]-32;
		}else if(ptr[i]>='A' && ptr[i]<='Z')
		{
			ptr[i]=ptr[i]+32;
		}
	}
}
static long my_ioctl(struct file*file,unsigned int cmd,unsigned long arg)
{
	if(cmd == CASE_CONVERT)
	{
		if(copy_from_user(str,(char  __user*) arg,SIZE))
			return -EFAULT;
		pr_info("Received string %s\n",str);
		transform(str);
		if(copy_to_user((char __user*)arg,str,SIZE))
			return -EFAULT;

	}else
	{
		return -EFAULT;
	}
return 0;
}
struct file_operations fops={
		.owner=THIS_MODULE,
		.unlocked_ioctl=my_ioctl
};

static int __init case_convert_init(void)
{
	major=register_chrdev(0,DEVICE_NAME,&fops);
	str=kmalloc(100*sizeof(char),GFP_KERNEL);
	if(!str)
	{
		printk(KERN_ERR"Failed to allocate memory\n");
		return -ENOMEM;
	}
	if(major<0)
	{
		printk(KERN_ALERT"Cannot create driver\n");
		return -EFAULT;
	}
	pr_info("case converter loaded major = %d \n",major);
	
return 0;
}
static void __exit case_convert_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("case converter unloaded");
	kfree(str);
}
module_init(case_convert_init);
module_exit(case_convert_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("chintubhai");
//MODULE_DESCRIPTION("Case converter using ioctl");
