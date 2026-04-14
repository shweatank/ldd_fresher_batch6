#include<linux/init.h>
#include<linux/module.h>
#include<linux/uaccess.h>
#include<linux/fs.h>
#include<linux/slab.h>
static int major;
#define MODULE_NAME "char_driver"
static char*ptr;
#define SIZE 100
static int my_open(struct inode*inode,struct file*file)
{
	printk(KERN_INFO"Driver opened\n");
	return 0;
}
static int my_release(struct inode*inode,struct file*file)
{
	printk(KERN_INFO"Driver Closed\n");
	return 0;
}
static ssize_t my_read(struct file *file,char __user*user_data,size_t count,loff_t *offset)
{
	int bytes_to_copy=min(count,(size_t)SIZE);
	if(copy_to_user((char __user*)user_data,ptr,bytes_to_copy))
		return -EFAULT;
	*offset+=bytes_to_copy;
return bytes_to_copy;
}
static ssize_t my_write(struct file*file,const char __user*user,size_t count,loff_t *offset)
{
	int bytes_to_copy=min(count,(size_t)SIZE);
	if(bytes_to_copy >=SIZE)
		return -EFAULT;	
	if(copy_from_user(ptr,(char __user*)user,bytes_to_copy))
		return -EFAULT;
return 0;
}

static struct file_operations fops={
				.owner=THIS_MODULE,
				.open=my_open,
				.read=my_read,
				.write=my_write,
				.release=my_release
};
static int __init my_init(void)
{
	major=register_chrdev(0,MODULE_NAME,&fops);
	printk(KERN_INFO"Major number %d\n",major);
	if(major<0)
	{
		printk(KERN_ERR"Failed to register module\n");
		return -EFAULT;
	}
	printk(KERN_INFO"Kernel MOdule loaded\n");
	ptr=(char *) kmalloc(SIZE*sizeof(char),GFP_KERNEL);
	if(!ptr)
	{
		printk(KERN_ALERT"Memory allocation failed\n");
		return -EFAULT;
	}	
return 0;
}
static void __exit my_exit(void)
{
	unregister_chrdev(major,MODULE_NAME);
	kfree(ptr);
	printk(KERN_INFO"Memory freed\n");
	printk(KERN_INFO"Module Unloaded\n");
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("Practicing chr driver");
