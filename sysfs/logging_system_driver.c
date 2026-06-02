#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/workqueue.h>
#include<linux/delay.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/wait.h>
#include<linux/timer.h>
#include<linux/delay.h>
#include<linux/device.h>
#include<linux/kthread.h>

#define DEVICE_NAME "mydevice"
#define LOG_SIZE 256

static int major_number;
static char kernel_buffer[BUF_SIZE];
static int buffer_size;
static wait_queue_head_t wq;
static int flag=0;

static struct hrtimer my_timer;
static ktime_t interval;
static struct work_struct my_work;

static struct task_struct *my_thread;

static enum hrtimer_restart timer_callback(struct hrtimer *t)
{
        pr_info("Timer fired\n");

        flag=1;

	schedule_work(&my_work);
       // wake_up_interruptible(&wq);

        hrtimer_forward_now(t,interval);
        return HRTIMER_RESTART;
}


//Work handler function
static void my_work_handler(struct work_struct *work)
{
	pr_info("Workqueue: Handler Started\n");
	int i;
	if (flag == 1)
	{
		for(i=0;i<buffer_size;i++)
		{
			if(kernel_buffer[i]>='a'&&kernel_buffer[i]<='z')
                	{
                        	kernel_buffer[i]=kernel_buffer[i]-32;
                	}
                	else if(kernel_buffer[i]>='A'&&kernel_buffer[i]<='Z')
                	{
                        	kernel_buffer[i]=kernel_buffer[i]+32;
                	}
                	else
                	{
                        	kernel_buffer[i]=kernel_buffer[i];
                	}
		}
		flag=0;
		msleep(2000);
                wake_up_interruptible(&wq);
        	pr_info("Workqueue: Handler Finished\n");
	}
}
static int thread_function(void *data)
{
	while(!kthread_should_stop())
	{
		pr_info("Kernel Thread running...\n");
		msleep(2000);
	}
	return 0;
}

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

static ssize_t basic_write(struct file *file,const char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy;
        bytes_to_copy = min(count, (size_t) BUF_SIZE);

        /*
         * Copy data from user space to kernel space
         */

        if(copy_from_user(kernel_buffer,user_buffer,bytes_to_copy))
                return -EFAULT;

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

//Module init
static int __init workq_init(void)
{
	pr_info("Initializing mydevice driver\n");

	major_number = register_chrdev(0,DEVICE_NAME,&basic_fops);
	if(major_number < 0)
        {
                printk(KERN_ERR "basic_char :failed to register device\n");
                return major_number;
        }

	//Initialize work
	INIT_WORK(&my_work, my_work_handler);
	init_waitqueue_head(&wq);

	interval=ktime_set(1,0);

	hrtimer_setup(&my_timer,timer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	hrtimer_start(&my_timer, interval , HRTIMER_MODE_REL);
	
	my_thread = kthread_run(thread_function,NULL,"my_kthread");
	//Schedule Work
	pr_info("Workqueue: Scheduling work\n");
       //schedule_work(&my_work);

	return 0;
}

//Module exit
static void __exit workq_exit(void)
{
	pr_info("Workqueue module exiting\n");

	//Ensure Work is completed before exit
	unregister_chrdev(major_number,DEVICE_NAME);
	flush_work(&my_work);
	hrtimer_cancel(&my_timer);

	if(my_thread)
	{
		kthread_stop(my_thread);
	}
	pr_info("Workqueue module unloaded\n");
}

module_init(workq_init);
module_exit(workq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ram");
MODULE_DESCRIPTION("Simple Example for WorkQueue in Linux");
