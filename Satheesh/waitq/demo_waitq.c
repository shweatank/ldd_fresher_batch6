#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/wait.h>
#include<linux/hrtimer.h>
#include<linux/ktime.h>

#define DEVICE_NAME "waitq_basic"
static int major;
static wait_queue_head_t wq;
static int flag=0;
static struct hrtimer my_timer;
static ktime_t interval;
/*Timer callback*/
static enum hrtimer_restart timer_callback(struct hrtimer*t)
{
	pr_info("Timer fired\n");
	flag=1;
	wake_up_interruptible(&wq);

	hrtimer_forward_now(t,interval);
	return HRTIMER_RESTART;
}
// Read blocks untill timer event 
static ssize_t my_read(struct file*file,char __user * buf,size_t len,loff_t *offset)
{
	char msg[]="Hello From Kernel\n";
	pr_info("Read:Waiting\n");
	wait_event_interruptible(wq,flag !=0);
	flag=0;
	copy_to_user(buf,msg,sizeof(msg));
	pr_info("Read: Done\n");
	return sizeof(msg);
}
static struct file_operations fops={.owner=THIS_MODULE,.read=my_read};
static int __init my_init(void)
{
	pr_info("Driver loaded\n");
	init_waitqueue_head(&wq);
	// Register device
	major=register_chrdev(0,DEVICE_NAME,&fops);
	printk("major number:%d\n",major);
	//Set up timer (1 sec)
	interval=ktime_set(1,0);
//	hrtimer_init(&my_timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	hrtimer_setup(&my_timer,timer_callback,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	my_timer.function=timer_callback;
	hrtimer_start(&my_timer,interval,HRTIMER_MODE_REL);
	return 0;
}
static void __exit my_exit(void)
{
	pr_info("Driver Unloaded\n");
	hrtimer_cancel(&my_timer);
	unregister_chrdev(major,DEVICE_NAME);
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("my waitq ");







