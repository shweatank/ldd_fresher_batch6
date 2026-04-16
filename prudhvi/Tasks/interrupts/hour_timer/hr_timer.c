#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/hrtimer.h>
#include<linux/ktime.h>

#define TIMER_INTERVAL_NS 1000000 //1 MS (1000000 NS)

static struct hrtimer my_hrtimer;
static ktime_t interval;

//hr timer callback

static enum hrtimer_restart hrtimer_callback(struct hrtimer *timer)
{
	pr_info("hrtimer fired!\n");

	//forward the timer to next period
	hrtimer_forward_now(timer,interval);

	return HRTIMER_RESTART; //KEEP IT PERIODIC
}

//module init
static int __init hrtimer_driver_init(void)
{
	pr_info("high resolution timer driver loaded\n");
	interval =ktime_set(0,TIMER_INTERVAL_NS);

	//initilizing hrtimer
	hrtimer_init(&my_hrtimer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	my_hrtimer.function=hrtimer_callback;

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
		pr_info("timer was active\n");
	pr_info("high resolution timer driver unloaded\n");
}
module_init(hrtimer_driver_init);
module_exit(hrtimer_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PRUDHVI");
MODULE_DESCRIPTION("LINUX KERNEL HIGH RESOLUTION TIMER EXAMPLE");
