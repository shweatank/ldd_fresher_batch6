#include<linux/module.h>
#include<linux/kobject.h>
#include<linux/sysfs.h>
#include<linux/init.h>
#include<linux/workqueue.h>
#include<linux/delay.h>

static struct work_struct my_work;
static struct kobject *demo_kobj;
static int num;
static wait_queue_head_t wq;
static int flag=0;

static struct hrtimer my_timer;
static ktime_t interval;
static enum hrtimer_restart timer_callback(struct hrtimer *t)
{
        pr_info("Tmer fired\n");

        schedule_work(&my_work);

        hrtimer_forward_now(t,interval);
        return HRTIMER_NORESTART;
}


static void my_work_handler(struct work_struct *work)
{
	pr_info("Workqueue: Handler started\n");
	num=num*2;
	flag=1;
        wake_up_interruptible(&wq);
	pr_info("Workqueue: Handler Finished\n");
}
//show function
static ssize_t value_show(struct kobject *kobj,struct kobj_attribute *attr, char *buf)
{
	wait_event_interruptible(wq, flag != 0);
	printk(KERN_INFO "value received %d\n",num);
	return sprintf(buf, "Result is  %d\n",num);
}

//Store function
static ssize_t value_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf,size_t count)
{
	sscanf(buf, "%d" ,&num);
	printk(KERN_INFO "value sent to driver %d\n",num);
	hrtimer_start(&my_timer, interval, HRTIMER_MODE_REL);
	return count;
}

static struct kobj_attribute value_attr =
		__ATTR(value, 0664, value_show, value_store);

static int __init sysfs_demo_init(void)
{
	demo_kobj= kobject_create_and_add("sysfs_demo", kernel_kobj);
	if(!demo_kobj)
		return -ENOMEM;
	init_waitqueue_head(&wq);
	INIT_WORK(&my_work, my_work_handler);
	interval = ktime_set(5,0);
	hrtimer_setup(&my_timer,timer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	sysfs_create_file(demo_kobj, &value_attr.attr);
	pr_info("sysfs_demo loaded\n");
	return 0;
}

static void __exit sysfs_demo_exit(void)
{
	kobject_put(demo_kobj);
	flush_work(&my_work);
	hrtimer_cancel(&my_timer);
	pr_info("sysfs_demo unloaded\n");
}

module_init(sysfs_demo_init);
module_exit(sysfs_demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple sysfs driver");
MODULE_AUTHOR("Ram");

