/*
 *1. Design a Linux device driver that simulates a virtual device where a user-space application writes a request (e.g., “generate data”) to /dev/mydevice.
The driver must:
  -> Simulate an interrupt (using a timer or software trigger) after receiving the request
  -> Handle the simulated interrupt in an ISR and defer work using a tasklet or workqueue
  -> Generate and store data inside the driver
  -> Put the user process to sleep using a waitqueue when it calls read() before data is ready
  -> Wake up the process once data is prepared
  -> Use a kernel thread to periodically generate or monitor data in the background
 * 
*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/hrtimer.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

#define DEVICE_NAME "My_DEVICE"

static int major;
static wait_queue_head_t wq;
static int flag = 0;

struct data_buffer_t {
       char buffer[1024];
       size_t size;
};

static struct data_buffer_t data_buffer = { .size = 0 };
static struct work_struct my_work;

#define TIMER_INTERVAL_NS 1000000 // 1 ms

static struct hrtimer my_hrtimer;
static ktime_t interval;

static void generate_data(void)
{
       for(int i = 0; i < 100; i++)
       {
	       data_buffer.buffer[i] = 'a';
       }
       data_buffer.buffer[100] = '\0';
}

// hr timer callback
static enum hrtimer_restart hrtimer_callback(struct hrtimer *timer)
{
        printk(KERN_INFO "My device: hrtimer fired!\n");

         schedule_work(&my_work);

        return HRTIMER_NORESTART;
}

// Work handler function
static void my_work_handler(struct work_struct *work)
{
        pr_info("Mydevice ,Work queue : Handler started\n");
        generate_data();
	flag = 1;
	wake_up_interruptible(&wq);

        pr_info("Mydevice , Workqueue : Handler finished\n");
}

static ssize_t my_read(struct file *file, char __user* buff, size_t count, loff_t *offset)
{
    size_t available_data = strlen(data_buffer.buffer);

    if (*offset >= available_data)
        return 0;

    if (wait_event_interruptible(wq, flag != 0))
        return -ERESTARTSYS; 

    if (count > available_data - *offset)
        count = available_data - *offset;

    if (copy_to_user(buff, data_buffer.buffer + *offset, count)) {
        return -EFAULT;
    }

    *offset += count;
    flag = 0; 

    return count;
}

static int my_strcmp(char *p, char *q)
{
	while(*p){
	  if(*p != *q) {	
                  printk(KERN_INFO "My device: not matched p=%c, q=%c\n",*p,*q);
		  return (*p - *q);
	  }
	  p++, q++;
	}
        printk(KERN_INFO "my device: String matched\n");
	return 0;
}

static ssize_t my_write(struct file *file,const char __user* buff, size_t count, loff_t *offset)
{
	char buffer[128];
        printk(KERN_INFO "My device : writing to kernel buffer\n");
	if(count > sizeof(buffer))
	    return -EFAULT;
	if(copy_from_user(buffer, buff, count))
		return -EFAULT;
    
        printk(KERN_INFO "My device : Received string -%s \n",buffer);
         if(my_strcmp(buffer, "Generate data\n") == 0){
         printk(KERN_INFO "My Device : starting... generatig data\n");
	       interval = ktime_set(0, TIMER_INTERVAL_NS); // 0 sec + ns
               hrtimer_start(&my_hrtimer, interval, HRTIMER_MODE_REL);
	 }
	 return count; 
}

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .read = my_read,
  .write = my_write,
};

static int __init my_init(void)
{
        printk(KERN_INFO "My device loaded\n");
        major = register_chrdev(0, DEVICE_NAME, &fops);
        printk(KERN_INFO "my device major number : %d\n",major);

        hrtimer_setup(&my_hrtimer, hrtimer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	//initialize some work
        INIT_WORK(&my_work, my_work_handler);

	init_waitqueue_head(&wq);

        //pr_info("Workqueue : Scheduling work");
        //schedule_work(&my_work);

        return 0;
}

//module exit
static void __exit my_exit(void)
{
        int ret;
        ret = hrtimer_cancel(&my_hrtimer);
        if(ret)
                printk(KERN_INFO "Timer was active\n");
        flush_work(&my_work);
        unregister_chrdev(major, DEVICE_NAME);
	printk(KERN_INFO "My device unloaded\n");
}


module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Almas");

