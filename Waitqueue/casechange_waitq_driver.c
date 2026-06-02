#include<linux/module.h>  //Core module macros
#include<linux/kernel.h> //printk()
#include<linux/init.h> // __init, __exit
#include<linux/fs.h>   //register _chrdev, file_operations
#include<linux/uaccess.h> //
#include<linux/hrtimer.h>
#include<linux/ktime.h>
#include<linux/wait.h>

#define DEVICE_NAME "casechange_waitq"
#define BUF_SIZE 256

static int major_number;
static char kernel_buffer[BUF_SIZE];
static int buffer_size;
static wait_queue_head_t wq;
static int flag=0;

static struct hrtimer my_timer;
static ktime_t interval;



//Timer Callback
static enum hrtimer_restart timer_callback(struct hrtimer *t)
{
	pr_info("Timer fired\n");

	flag=1;

	wake_up_interruptible(&wq);

	hrtimer_forward_now(t,interval);
	return HRTIMER_RESTART;
}


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
	return 0;
}

/*
 * called when user reads from /dev/basic_char
 */

static ssize_t basic_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{

	pr_info("Read: waiting...\n");

	wait_event_interruptible(wq, flag != 0);

	flag=0;

	int bytes_to_copy;

	/*
	 * If offset is beyond data, return 0 (EOF)
	 */
	if(*offset >= buffer_size)
		return 0;

	bytes_to_copy = min(count, (size_t)(buffer_size- *offset));

	/*
	 * Copy data from kernel space to user space
	 */
	if(copy_to_user(user_buffer,kernel_buffer + *offset,bytes_to_copy))
		return -EFAULT;

	*offset += bytes_to_copy;

	printk(KERN_INFO "Read: done. read %d bytes\n",bytes_to_copy);
	return bytes_to_copy;
}

/*
 * Called when user writes to /dev/basic_char
 */

static ssize_t basic_write(struct file *file,const char __user *user_buffer, size_t count, loff_t *offset)
{
	int bytes_to_copy;
	bytes_to_copy = min(count, (size_t) BUF_SIZE-1);

	/*
	 * Copy data from user space to kernel space
	 */
	char temp_buffer[BUF_SIZE];
	if(copy_from_user(temp_buffer,user_buffer,bytes_to_copy))
		return -EFAULT;
	
	temp_buffer[bytes_to_copy]='\0';
	int i;
	for(i=0;i<bytes_to_copy;i++)
	{
		if(temp_buffer[i]>='a'&&temp_buffer[i]<='z')
		{
			kernel_buffer[i]=temp_buffer[i]-32;
		}
		else if(temp_buffer[i]>='A'&&temp_buffer[i]<='Z')
                {
                        kernel_buffer[i]=temp_buffer[i]+32;
                }
		else
		{
			kernel_buffer[i]=temp_buffer[i];
		}
	}
	kernel_buffer[i]='\0';
	buffer_size = bytes_to_copy;

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
	pr_info("Driver : Loaded\n");
	major_number = register_chrdev(0,DEVICE_NAME,&basic_fops);
	if(major_number < 0)
	{
		printk(KERN_ERR "device :failed to register device\n");
		return major_number;
	}

	init_waitqueue_head(&wq);

	interval = ktime_set(1,0);

	hrtimer_setup(&my_timer,timer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	hrtimer_start(&my_timer, interval , HRTIMER_MODE_REL);
	
	return 0;
}

/*Module Cleanup
 */

static void __exit basic_char_exit(void)
{
	hrtimer_cancel(&my_timer);
	unregister_chrdev(major_number, DEVICE_NAME);
	printk(KERN_INFO "Driver : unloaded\n");
}

/*Kernel module macros*/
module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ram");
MODULE_DESCRIPTION("CaseChange using WaitQueue");

