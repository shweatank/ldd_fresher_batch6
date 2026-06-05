#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#define TIMER_INTERVAL_MS 1000 // 1 SECOND

static struct timer_list my_timer;

//timer callback function

static void my_timer_callback(struct timer_list *t)
{
	printk(KERN_INFO "Timer interrupt occurred!\n");
	//restart the timer (periodic behaviour)
	
	mod_timer(&my_timer,jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));
}

// module intilization

static int __init timer_driver_init(void)
{
	printk(KERN_INFO "timer driver loaded\n");

	//intilize timer
	timer_setup(&my_timer, my_timer_callback,0);
	//start timer
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));
	return 0;
}
// module exit
static void __exit timer_driver_exit(void)
{
	timer_delete_sync(&my_timer);
	printk(KERN_INFO"timer driver unloaded\n");
}

module_init(timer_driver_init);
module_exit(timer_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pavan");
MODULE_DESCRIPTION("simple linux kernel timer driver");
