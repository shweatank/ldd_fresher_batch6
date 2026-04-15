#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include<linux/workqueue.h>
#include<linux/delay.h>
#include<linux/kthread.h>
#include<linux/timer.h>
#include<linux/wait.h>

#define DEVICE_NAME "deferred_logger"

static int major;

static char log_buffer[256];
static char temp_buffer[256];
static int data_ready=0;

static wait_queue_head_t wq;

static struct work_struct my_work;
static struct timer_list my_timer;
static struct task_struct *my_thread;
/*work handler function*/
static void my_work_handler(struct work_struct *work)
{
        pr_info("Workqueue:Formatting log\n");
	snprintf(log_buffer,sizeof(log_buffer),"LOG:%s",temp_buffer);
	data_ready=1;
        wake_up_interruptible(&wq);
}

static void timer_callback(struct timer_list *t)
{
	pr_info("ISR:software interrupt triggered\n");
	schedule_work(&my_work);
}
/*ioctl handler */
static ssize_t my_write(struct file *file,const char __user *buf,size_t len,loff_t *off)
{
	if(len>sizeof(temp_buffer))
		len=sizeof(temp_buffer);
	if(copy_from_user(temp_buffer,buf,len))
	{
		return -EFAULT;
	}
	pr_info("Write:Recieved log request\n");
	mod_timer(&my_timer,jiffies+msecs_to_jiffies(1000));
	return len;
}

static ssize_t my_read(struct file *file,char __user *buf,size_t len,loff_t *off)
{
	pr_info("Read:waiting for log..\n");
	wait_event_interruptible(wq,data_ready==1);
        data_ready=0;
        if(copy_to_user(buf,log_buffer,strlen(log_buffer)+1))
        {
                return -EFAULT;
        }
        pr_info("Read:log sent\n");
        return strlen(log_buffer)+1;
}
static int thread_fn(void *data)
{
	while(!kthread_should_stop())
	{
		if(data_ready)
		{
			pr_info("Kernel Thread flush:%s\n",log_buffer);
		}
		ssleep(5);
	}
	return 0;
}
static struct file_operations fops = {
        .owner = THIS_MODULE,
        .write=my_write,
	.read=my_read,
};

static int __init basic_init(void)
{
        major = register_chrdev(0, DEVICE_NAME, &fops);
	pr_info("Driver loaded,major=%d\n",major);
        /*initialize work*/
	init_waitqueue_head(&wq);
        INIT_WORK(&my_work,my_work_handler);
	timer_setup(&my_timer,timer_callback,0);
	my_thread=kthread_run(thread_fn,NULL,"my_kthread");
	return 0;
}
static void __exit basic_exit(void)
{
	add_timer(&my_timer);
	flush_work(&my_work);
	if(my_thread)
		kthread_stop(my_thread);
        unregister_chrdev(major,DEVICE_NAME);
        pr_info("Driver unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sarika");
MODULE_DESCRIPTION("workqueue linux kernel");
