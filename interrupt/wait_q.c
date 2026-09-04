#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/wait.h>
#include<linux/hrtimer.h>
#include<linux/ktime.h>

#define DEVICE_NAME "waitq_basic"

static int major_number;
static wait_queue_head_t wq;
static int flag = 0;


static struct hrtimer my_timer;
static ktime_t interval;


//Timer callback
static enum hrtimer_restart timer_callback(struct hrtimer *t)
{
	pr_info("Timer fired");

	//set the flag 
	flag = 1;

	//wake up the workqueue as the flag is set
	wake_up_interruptible(&wq);

	hrtimer_forward_now(t, interval);
	return HRTIMER_RESTART;

}

/*Read :  blocks until the timer event*/
static ssize_t my_read(struct file *file , char __user *buf, size_t len , loff_t *off)
{
	char msg[] = "Hello from kernel\n";

	pr_info("Read : waiting\n");   //here now read wait till the flag value is set to 1 by the timer

	//this is because here the wait queue is interrupted using the flag value using wait_event_interruptible 

	wait_event_interruptible(wq , flag != 0);

	flag = 0;

	if(copy_to_user(buf , msg , sizeof(msg)))
	{
		return -EFAULT;
	}

	pr_info("Read done\n");

	return sizeof(msg);
}

static struct file_operations fops ={
	.owner = THIS_MODULE , 
	.read = my_read,
};

//module init
static int __init my_init(void)
{
	pr_info("Driver loaded\n");

	//initializing the wait queue
	init_waitqueue_head(&wq);

	//Register device
	major_number = register_chrdev(0 , DEVICE_NAME , &fops);

	//setup_timer (1s)
	interval = ktime_set(1 , 0);
	printk(KERN_INFO"The major number %d\n",major_number);
	//initializing hrtimer
	hrtimer_setup(&my_timer, timer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	hrtimer_start(&my_timer,interval,HRTIMER_MODE_REL);
	return 0;

}

//exit
static void __exit my_exit(void)
{
	pr_info("Driver unloaded\n");

	//cancelling the timer
	hrtimer_cancel(&my_timer);

	//unregister the device file
	unregister_chrdev(major_number , DEVICE_NAME);

}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GANGADHAR");
MODULE_DESCRIPTION("This is a basic linux device driver to waitqueue\n");
