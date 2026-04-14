#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/hrtimer.h>
#include<linux/ktime.h>

#define TIMER_INTERVAL_NS 1000000  // 1Ms (1,000,000 ns)

static struct hrtimer my_hrtimer;
static ktime_t interval;

/* hrtimer callback */
static enum hrtimer_restart hrtimer_callback(struct hrtimer *timer){
  printk(KERN_INFO "hrtimer fired!\n");

  /*Forward the timer to next period*/
  hrtimer_forward_now(timer,interval);

  return HRTIMER_RESTART; // Keep it Periodic
}
/* Module init */
static int __init hrtimer_driver_init(void){
  printk(KERN_INFO "High-Resolution timer driver loaded\n");
  interval = ktime_set(0,TIMER_INTERVAL_NS); //0 SEC + ns

  /* Initialize hrtimer */
  // hrtimer_init(&my_hrtimer,CLOCK_MONOTONIC , HRTIMER_MODE_REL); 
  // my_hrtimer.function = hrtimer_callback;
  
  hrtimer_setup(&my_hrtimer, hrtimer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

  /*start timer*/
  hrtimer_start(&my_hrtimer,interval, HRTIMER_MODE_REL); 
  return 0;
}
static void __exit hrtimer_driver_exit(void){
  int ret;
  ret=hrtimer_cancel(&my_hrtimer);
  if(ret)
	  printk(KERN_INFO "Timer was active\n");

  printk(KERN_INFO "High-Resolution timer driver unloaded\n");
}
module_init(hrtimer_driver_init);
module_exit(hrtimer_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rahul");
MODULE_DESCRIPTION("Linux Kernel High-Resolution Timer Example");

