#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/timer.h>
#include<linux/jiffies.h>

#define TIMER_INTERVAL_MS 1000 //1 SEC

static struct timer_list my_timer;

//timer callback function
static void my_timer_callback(struct timer_list *t)
{
	pr_info("Timer interrupt occurred\n");
	//restart the timer periadic behevior
	mod_timer(&my_timer,jiffies +msecs_to_jiffies(TIMER_INTERVAL_MS));
}

//module initilization
static int __init timer_driver_init(void)
{
	pr_info("Timer driver loaded\n");
	//initializing timer
	timer_setup(&my_timer,my_timer_callback,0);
	//start timer
	mod_timer(&my_timer,jiffies +msecs_to_jiffies(TIMER_INTERVAL_MS));
	return 0;
}
//module exit
static void __exit timer_driver_exit(void)
{
	del_timer_sync(&my_timer);
	pr_info("Timer driver unloaded\n");

}
module_init(timer_driver_init);
module_exit(timer_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PRUDHVI");
MODULE_DESCRIPTION("SIMPLE LINUX KERNEL TIMER INTERRUPT");

