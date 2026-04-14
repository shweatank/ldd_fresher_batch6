#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/timer.h>
#include<linux/jiffies.h>

#define TIMER_INTERVAL_MS 1000 //1 SECOND

static struct timer_list my_timer;
/*Timer call back function*/
static void my_timer_callback(struct timer_list *t){
 printk(KERN_INFO "Timer interrupt occurred!!\n");

 /*Restart the timer (periodic behaviour)*/
 mod_timer(&my_timer,jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));
}

/*MODULE INITIALIZATION*/
static int __init timer_driver_init(void){
 printk(KERN_INFO "Timer interrupt loaded!!\n");
 /*Initiate timer*/
 timer_setup(&my_timer,my_timer_callback,0);
 /*start timer*/
 mod_timer(&my_timer,jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));
 return 0;
}
/*MODULE exit*/
static void __exit timer_driver_exit(void){
 timer_delete_sync(&my_timer);
 printk(KERN_INFO "Timer driver unloaded!!\n");
}
module_init(timer_driver_init);
module_exit(timer_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("RAHUL");
MODULE_DESCRIPTION("Simple Linux Kernel Timer Driver");
