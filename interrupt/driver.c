/*Design a Linux device driver that simulates a virtual device where a user-space application writes a request (e.g., “generate data”) to /dev/mydevice.
The driver must:
Simulate an interrupt (using a timer or software trigger) after receiving the request
Handle the simulated interrupt in an ISR and defer work using a tasklet or workqueue
Generate and store data inside the driver
Put the user process to sleep using a waitqueue when it calls read() before data is ready
Wake up the process once data is prepared
Use a kernel thread to periodically generate or monitor data in the background*/
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/wait.h>
#include<linux/hrtimer.h>
#include<linux/ktime.h>
#include <linux/workqueue.h>
#include <linux/delay.h>


#define DEVICE_NAME "driver"
#define BUF_SIZE 256

static int major_number;
static char kernel_buffer[BUF_SIZE];
static int buffer_size;

static wait_queue_head_t wq;
static int flag = 0;

static struct hrtimer my_timer;
static ktime_t interval;

static struct work_struct my_work;

static struct task_struct *my_thread;

static void my_work_handler(struct work_struct *work)
{
    pr_info("Work queue : Handler started\n");

    msleep(1000);
    	
     snprintf(kernel_buffer, BUF_SIZE, "DATA READY from driver\n")
     buffer_size = strlen(kernel_buffer);
  
        flag  = 1;
  
          pr_info("Workqueue: data generated\n");
  
          wake_up_interruptible(&wq);

    pr_info("Work queue: Handler finished\n");
}


static enum hrtimer_restart timer_callback(struct hrtimer *t)
{
	pr_info("Timer fired");


	//wake up the workqueue as the flag is set
	//wake_up_interruptible(&wq);

	schedule_work(&my_work);
	return HRTIMER_NORESTART;

}


static int thread_fn(void *data)
  {
          //static int i =0;
          while (!kthread_should_stop())
          {
                  pr_info("Kernel thread running...\n");
                  ssleep(5);
          }
          return 0;
 }

static int basic_open(struct inode *inode,struct file *file)
{
printk(KERN_INFO "basic_char: device opened\n");
return 0;
}

/*
*called when user closes dev/nasic_char
*/

static int basic_release(struct inode *inode,struct file *file)
{
printk(KERN_INFO "basic_char: device closed\n");
return 0;
}
static ssize_t basic_read(struct file *file,char __user *user_buffer,size_t count,loff_t *offset)
{
int bytes_to_copy;
/*
if offset is beyond data, return 0 (EOF)
*/

if(*offset >= buffer_size)
return 0;

bytes_to_copy = min(count,(size_t)(buffer_size - *offset));

/*
copy data from kernel space to user space
*/
wait_event_interruptible(wq , flag != 0);

	flag = 0;
if(copy_to_user(user_buffer,kernel_buffer + *offset,bytes_to_copy))
return -EFAULT;

*offset += bytes_to_copy;

printk(KERN_INFO "basic_char: read %d bytes\n",bytes_to_copy);
printk(KERN_INFO"%s\n",kernel_buffer);
return bytes_to_copy;
}

/*
called when user writes to dev/basic_char
*/

static ssize_t basic_write(struct file *file,const char __user *user_buffer,size_t count,loff_t *offset)
{
int bytes_to_copy;

bytes_to_copy = min(count,(size_t)BUF_SIZE);
/*
copy data from the user space to kernel space 
*/

if(copy_from_user(kernel_buffer,user_buffer,bytes_to_copy))
return -EFAULT;

buffer_size = bytes_to_copy;
kernel_buffer[buffer_size] = '/0';

if(strcmp(kenerl_buffer,"generate data") == 0)
{
hrtimer_start(&my_timer,interval,HRTIMER_MODE_REL);

}
printk(KERN_INFO "basic_char: wrote %d bytes\n",bytes_to_copy);
return bytes_to_copy;
}

static struct file_operations basic_fops = {
.owner = THIS_MODULE,
.open = basic_open,
.read = basic_read,
.write = basic_write,
.release = basic_release
};

/*
Module intialiazation
*/

static int __init basic_char_init(void)
{
/*
register char device
0 ---> dynamic major number
*/

major_number = register_chrdev(0,DEVICE_NAME,&basic_fops);
if(major_number < 0)
{
printk(KERN_ERR "basic_char: failed to register device\n");
return major_number;
}
init_waitqueue_head(&wq);
interval = ktime_set(1 , 0);
hrtimer_setup(&my_timer, timer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

my_thread = kthread_run(thread_fn, NULL, "my_kthread");

//hrtimer_start(&my_timer,interval,HRTIMER_MODE_REL);

INIT_WORK(&my_work, my_work_handler);


printk(KERN_INFO "basic_char: loaded\n");
printk(KERN_INFO "basic_char: major  number = %d\n",major_number);
printk(KERN_INFO "create device node with:\n");
printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);
return 0;
}

/*
module cleanup
*/

static void __exit basic_char_exit(void)
{
  hrtimer_cancel(&my_timer);
      flush_work(&my_work);
 
 
         if (my_thread)
            kthread_stop(my_thread);


unregister_chrdev(major_number,DEVICE_NAME);
printk(KERN_INFO "basic char: unloaded\n");
}

/*
kernel module macros
*/

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gangadhar");
MODULE_DESCRIPTION("Educational basic character driver with file operation");
