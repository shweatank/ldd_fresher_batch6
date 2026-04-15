	#include<linux/module.h>
	#include<linux/kernel.h>
	#include<linux/fs.h>
	#include<linux/uaccess.h>
	#include<linux/init.h>
	#include<linux/wait.h>
	#include<linux/hrtimer.h>
	#include<linux/ktime.h>
	#include<linux/slab.h>
	#include<linux/interrupt.h>
	#define DEVICE_NAME "waitq_basic"
	char *ptr;
	static int major;
	static wait_queue_head_t wq;
	static int flag=0;
	static struct hrtimer my_timer;
	static ktime_t interval;
	//static struct work_struct my_work;
	static int num=100;
	static void tasklet_fun(struct tasklet_struct *);
	DECLARE_TASKLET(virtual_tasklet,tasklet_fun);
	static void tasklet_fun(struct tasklet_struct *t)
	{
		num+=100;
		flag=1;
		wake_up_interruptible(&wq);
	}
	/*	static void my_work_handler(struct work_struct *work)
	{
		pr_info("Assigned to the work queue\n");
		printk(KERN_INFO"Received string: %s\n",ptr);
	}*/
	/*Timer callback*/
	static enum hrtimer_restart timer_callback(struct hrtimer*t)
	{
		pr_info("Timer fired\n");
		tasklet_schedule(&virtual_tasklet);
		return HRTIMER_NORESTART;
	}
	// Read blocks untill timer event 
	static ssize_t my_read(struct file*file,char __user * buf,size_t len,loff_t *offset)
	{
		
		//hrtimer_forward_now(t,interval);
		hrtimer_start(&my_timer,interval,HRTIMER_MODE_REL);
		char msg[]="Hello From Kernel\n";
		pr_info("Read:Waiting\n");
		wait_event_interruptible(wq,flag !=0);
		flag=0;
		copy_to_user((int __user*)buf,&num,sizeof(int));
		pr_info("Read: Done\n");
		return sizeof(msg);
	}

/*	static ssize_t my_write(struct file *file,const char __user *buf , size_t count , loff_t *offset)
	{
		pr_info("Write:Waiting Form kernel\n");
		wait_event_interruptible(wq,flag!=0);
		flag=0;
		copy_from_user(ptr,buf,20);
		schedule_work(&my_work);
		return count;
	}*/

	static struct file_operations fops={.owner=THIS_MODULE,.read=my_read};
	static int __init my_init(void)
	{
		/*ptr=(char *) kmalloc(20*sizeof(char),GFP_KERNEL);
		if(!ptr)
		{
			pr_err("Memory allocation failed\n");
			return -ENOMEM;
		}*/
		pr_info("Driver loaded\n");
		init_waitqueue_head(&wq);
		// Register device
		major=register_chrdev(0,DEVICE_NAME,&fops);
		printk("major number:%d\n",major);
		//Set up timer (1 sec)
		interval=ktime_set(1,0);
	//	hrtimer_init(&my_timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
		hrtimer_setup(&my_timer,timer_callback,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
		//my_timer.function=timer_callback;
		//INIT_WORK(&my_work,my_work_handler);
		return 0;
	}
	static void __exit my_exit(void)
	{
		pr_info("Driver Unloaded\n");
		hrtimer_cancel(&my_timer);
		unregister_chrdev(major,DEVICE_NAME);
		//flush_work(&my_work);
		tasklet_kill(&virtual_tasklet);
		//kfree(ptr);
	}
	module_init(my_init);
	module_exit(my_exit);
	MODULE_LICENSE("GPL");
	MODULE_AUTHOR("Satheesh");
	MODULE_DESCRIPTION("my waitq ");







