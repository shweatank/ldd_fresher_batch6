#include<linux/module.h>  //Core module macros
#include<linux/kernel.h> //printk()
#include<linux/init.h> // __init, __exit
#include<linux/fs.h>   //register _chrdev, file_operations
#include<linux/uaccess.h> //

#define DEVICE_NAME "sort_driver"
#define BUF_SIZE 256

static int major_number;
struct Operation
{
	int size;
	int arr[];
};
struct Operation *k_buffer;
/*
 * called when user opens /dev/badic_char
 */
static int basic_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "basic_char: device opened\n");
	return 0;
}

/* called when user closes /dev/basic_char
 */

static int basic_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "basic_char: device closed\n");
	kfree(k_buffer);
	return 0;
}

/*
 * called when user reads from /dev/basic_char
 */

static ssize_t basic_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
	//int bytes_to_copy;

	/*
	 * If offset is beyond data, return 0 (EOF)
	 */
	//if(*offset >= buffer_size)
	//	return 0;

	/*
	 * Copy data from kernel space to user space
	 */
	if(copy_to_user(user_buffer,k_buffer,count))
		return -EFAULT;

	//*offset += bytes_to_copy;

	printk(KERN_INFO "basic_char: read %d bytes\n",count);
	return count;
}

/*
 * Called when user writes to /dev/basic_char
 */

static ssize_t basic_write(struct file *file,const char __user *user_buffer, size_t count, loff_t *offset)
{
	int bytes_to_copy=count; 
	/*
	 * Copy data from user space to kernel space
	 */
	k_buffer=kmalloc(count, GFP_KERNEL);
	if(copy_from_user(k_buffer,user_buffer,count))
		return -EFAULT;
	int size=k_buffer->size;
	for(int i=0;i<size-1;i++)
	{
		for(int j=0;j<size-i-1;j++)
		{
			if(k_buffer->arr[j]>k_buffer->arr[j+1])
			{
				int temp=k_buffer->arr[j];
				k_buffer->arr[j]=k_buffer->arr[j+1];
				k_buffer->arr[j+1]=temp;
			}
		}
	}
//	buffer_size =bytes_to_copy;
	
	printk(KERN_INFO "basic_char: wrote %d bytes\n",bytes_to_copy);
	return bytes_to_copy;
}

/*
 * File Operations Structure
 * This Connects System Calls to driver functions
 */

static struct file_operations basic_fops = {
	.owner = THIS_MODULE,
	.open = basic_open,
	.read = basic_read,
	.write = basic_write,
	.release = basic_release,
};

/*
 * Module Initilization
 */

static int __init basic_char_init(void)
{
	/*
	 * Register Character Device
	 * 0 -> dynamic major number
	 */

	major_number = register_chrdev(0,DEVICE_NAME,&basic_fops);
	if(major_number < 0)
	{
		printk(KERN_ERR "basic_char :failed to register device\n");
		return major_number;
	}
	printk(KERN_INFO "basic_char: loaded\n");
	printk(KERN_INFO "basic_char: major number = %d\n",major_number);
	printk(KERN_INFO "Create device node with:\n");
	printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME, major_number);

	return 0;
}

/*Module Cleanup
 */

static void __exit basic_char_exit(void)
{
	unregister_chrdev(major_number, DEVICE_NAME);
	printk(KERN_INFO "basic_char: unloaded\n");
}

/*Kernel module macros*/
module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ram");
MODULE_DESCRIPTION("Educational basics character driver with file operations");

