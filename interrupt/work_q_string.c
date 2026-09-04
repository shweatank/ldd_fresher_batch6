#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include<linux/fs.h>
#include<linux/uaccess.h>

#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256

static int major_number;
static char kernel_buffer[BUF_SIZE];
static int buffer_size;


static struct work_struct my_work;

static void my_work_handler(struct work_struct *work)
{
	pr_info("Work queue : Handler started\n");

	msleep(1000);


	for (int i = 0; i < buffer_size; i++) {
		if (kernel_buffer[i] >= 'a' && kernel_buffer[i] <= 'z') 
		{
			kernel_buffer[i] -= 32; 
		}
		else if(kernel_buffer[i] >= 'A' && kernel_buffer[i] <= 'Z')
		{
			kernel_buffer[i] += 32;
		}
	}
	pr_info("Work queue: Handler finished\n");
}
static int basic_open(struct inode *inode,struct file *file)
{
	printk(KERN_INFO "basic_char: device opened\n");
	return 0;
}

/*
 *called when user closes dev/nasic_char
 */

static int basic_release(struct inode *inode,struct file *file)
{
	printk(KERN_INFO "basic_char: device closed\n");
	return 0;
}

static ssize_t basic_read(struct file *file,
		char __user *user_buffer,
		size_t count,
		loff_t *offset)
{
	int bytes_to_copy;

	/* EOF condition */
	if (*offset >= buffer_size)
		return 0;

	/* Determine how many bytes to copy */
	bytes_to_copy = min(count, (size_t)(buffer_size - *offset));

	if (copy_to_user(user_buffer,
				kernel_buffer + *offset,
				bytes_to_copy))
		return -EFAULT;

	*offset += bytes_to_copy;

	printk(KERN_INFO "basic_char: read %d bytes\n", bytes_to_copy);

	return bytes_to_copy;
}

static ssize_t basic_write(struct file *file,const char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy;


	/* Copy only up to buffer size */
	bytes_to_copy = min(count, (size_t)BUF_SIZE);

	if (copy_from_user(kernel_buffer, user_buffer, bytes_to_copy))
		return -EFAULT;
	bytes_to_copy = buffer_size;
	schedule_work(&my_work);  // trigger processing 
	printk(KERN_INFO "basic_char: wrote %d bytes\n",bytes_to_copy);
	schedule_work(&my_work);
	return bytes_to_copy;

}



static struct file_operations basic_fops = {
	.owner = THIS_MODULE,
	.open = basic_open,
	.read = basic_read,
	.write = basic_write,
	.release = basic_release
};


static int __init workq_init(void)
{
	pr_info("Workqueue module loaded\n");

	// initialize work
	INIT_WORK(&my_work, my_work_handler);

	// scheduling work
	pr_info("Workqueue: Scheduling work\n");

	schedule_work(&my_work);
	major_number = register_chrdev(0,DEVICE_NAME,&basic_fops);
	if(major_number < 0)
	{
		printk(KERN_ERR "basic_char: failed to register device\n");
		return major_number;
	}

	printk(KERN_INFO "basic_char: loaded\n");
	printk(KERN_INFO "basic_char: major  number = %d\n",major_number);
	printk(KERN_INFO "create device node with:\n");
	printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);

	return 0;
}

static void __exit workq_exit(void)
{
	pr_info("Workqueue module exiting\n");

	flush_work(&my_work);

	pr_info("Workqueue module unloaded\n");

	unregister_chrdev(major_number,DEVICE_NAME);
	printk(KERN_INFO "basic char: unloaded\n");

}

module_init(workq_init);
module_exit(workq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gangadhar");
MODULE_DESCRIPTION("Sample WORK QUEUE module");
