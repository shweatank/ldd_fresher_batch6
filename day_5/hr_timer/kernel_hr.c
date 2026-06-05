#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#define TIMER_INTERVAL_NS 1000000 // 1 ms (1,000,000 ns)

static struct hrtimer my_hrtimer;
static ktime_t interval;

//hrtimer callback
static enum hrtimer_restart hrtimer_callback(struct hrtimer *timer)
{
	printk(KERN_INFO "hrtimer fireed\n");
	//forward the timer to next period
	hrtimer_forward_now(timer,interval);

	return HRTIMER_RESTART; //keep it periodic

}

//module init

static int __init hrtimer_driver_init(void)
{
	printk(KERN_INFO "high resolution timer driver loaded\n");

	interval = ktime_set(0,TIMER_INTERVAL_NS); //0 sec + ns

	//initialize hrtimer
	hrtimer_setup(&my_hrtimer,hrtimer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	my_hrtimer.function = hrtimer_callback;

	//start timer
	hrtimer_start(&my_hrtimer,interval,HRTIMER_MODE_REL);

	return 0;
}

//module exit

static void __exit hrtimer_driver_exit(void)
{
	int ret;
	ret=hrtimer_cancel(&my_hrtimer);
	if(ret)
		printk(KERN_INFO "timer is active\n");

	printk(KERN_INFO "high resolution timer driver unloaded\n");
}

module_init(hrtimer_driver_init);
module_exit(hrtimer_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pavan");
MODULE_DESCRIPTION("linux kernel high resolution timer example");
