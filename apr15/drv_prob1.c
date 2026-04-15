#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#define TIMER_INTERVAL_MS 1000 //1 second

static struct timer_list my_timer;

/*timer callback function*/
static void my_timer_callback(struct timer_list *t)
{
	printk(KERN_INFO "Timer interrupt occurred\n");

	/*Restart the timer (periodic behavior)*/
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));
}
static int __init my_init(void)
{
	pr_info("Driver loaded\n");
	init_waitqueue_head(&wq);
	
	/*register the device*/
	major=register_chrdev(0,DEVICE_NAME,&fops);
	if(major<0)
	{
		pr_info("Failed register\n");
		return major;
	}
	pr_info("basic_device loaded,major=%d\n",major);
	INIT_WORK(&my_work,my_work_handler);
	timer_setup(&my_timer,my_timer_callback,0);
	return 0;
}
static void __exit my_exit(void)
{
	pr_info("Driver unloaded\n");
	unregister_chrdev(major,DEVICE_NAME);
	timer_delete_sync(&my_timer);

