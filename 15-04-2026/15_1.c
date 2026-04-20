/* Device driver that simulates  a virtual device where a user_space application writes a request data(eg:generate data)
   to /dev/15_1.
   The driver must:
   Simulate an interrupt(using a timer)after receiving the request
   Handle the simulated interrupt in an ISR(timer fn) and defer work using workqueue
   Generate and store data inside the driver.
   Put the user process to sleep using  waitqueue when it calls read() before data is ready
   Wake up the process once data is ready */


#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/init.h>
#include<linux/uaccess.h>

#include<linux/timer.h>
#include<linux/jiffies.h>

#include<linux/workqueue.h>

#include<linux/wait.h>


#define DEVICE_NAME "15_1"
#define TIMER_INTERVAL_MS 2000  //1 SEC
#define BUFFER_SIZE 256

static char msg[BUFFER_SIZE];
static int major=0;
static struct timer_list my_timer;
static struct work_struct my_work;
static wait_queue_head_t wq;
static int cnt=1000;
static int work_done=0;

/*Timer callback function*/

static void my_timer_callback(struct timer_list  *t)
{
        printk(KERN_INFO "Timer interrupt occured!\n");

        /* Schedule work */
        pr_info("Workqueue: Scheduling work\n");
        schedule_work(&my_work);


         /*Restart the timer (periodic behaviour) */
        mod_timer(&my_timer,jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));

}


/*work handler function */
static void my_work_handler(struct work_struct *work)
{
        pr_info("Workqueue: Handler started\n");

        snprintf(msg,BUFFER_SIZE,"%d",cnt++);

        pr_info("Workqueue: Handler finished\n");

        work_done=1;
        wake_up_interruptible(&wq);
}


static ssize_t my_read(struct file *file,char __user *buf,size_t len,loff_t *off)
{

        pr_info("Read: Waiting...\n");

        wait_event_interruptible(wq,work_done!=0);

        int bytes_to_copy=min(len,(size_t)BUFFER_SIZE);


        if(copy_to_user(buf,msg+(*off),bytes_to_copy))
        {
                return -EFAULT;
        }
        pr_info("Read: done\n");

        *off=0;

        work_done=0;

        return bytes_to_copy;;
}

static ssize_t my_write(struct  file *file,const char __user *buf,size_t len,loff_t *off)
{
        pr_info("write initiated\n");

         /* Start timer */
        mod_timer(&my_timer,jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));

        return len;
}


static int basic_open(struct inode *inode,struct file *file)
{
        printk(KERN_INFO "waitqueue_2: device opened\n");
        return 0;
}

/*
   *called when user closes /dev/waitqueue_2
   */

static int basic_release(struct inode *inode,struct file *file)
{
        printk(KERN_INFO "waitqueue_2: device close\n");
        return 0;
}


static struct file_operations fops={
        .owner=THIS_MODULE,
        .read=my_read,
        .write=my_write,
        .open=basic_open,
        .release=basic_release,
};

/* INIT */

static int __init my_init(void)
{
        pr_info("Driver loaded\n");
        /* Register device */
        major=register_chrdev(0,DEVICE_NAME,&fops);
        printk(KERN_INFO "major number=%d\n",major);


        printk(KERN_INFO "Timer driver loaded\n");
        /*initialize timer */
        timer_setup(&my_timer,my_timer_callback,0);


         pr_info("workqueue module loaded\n");
        /* Initalize work */
        INIT_WORK(&my_work,my_work_handler);


        printk(KERN_INFO "waitqueue initialization done\n");
        init_waitqueue_head(&wq);

        return 0;
}

/* Exit */

static void __exit my_exit(void)
{
        pr_info("Driver unloaded\n");
        unregister_chrdev(major,DEVICE_NAME);



         pr_info("Workqueue module exiting\n");

        /* Ensure work is completed before exit */
        flush_work(&my_work);

        pr_info("Workqueue module unloaded\n");



        del_timer_sync(&my_timer);
        printk(KERN_INFO "Timer driver unloaded\n");

}


module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Techdhaba");
MODULE_DESCRIPTION("waitqueue_2 waiting for user input");






