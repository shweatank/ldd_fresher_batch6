/* The driver for :
   * First read succeeds only after first write
   *Subsequent reads succeed periodically via the timer
   *If the data becomes empty after several reads,the
    next read should block again until another write happens  */

#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/wait.h>
#include<linux/hrtimer.h>
#include<linux/ktime.h>

#define DEVICE_NAME "waitqueue_2"
#define BUFFER_SIZE 256

static int major=0;
static wait_queue_head_t wq;
static int flag=0;
static int write_done=0;
static char msg[BUFFER_SIZE];

static struct hrtimer my_timer;
static ktime_t interval;

/*Timer callback */

static enum hrtimer_restart timer_callback(struct hrtimer *t)
{
        pr_info("Timer fired\n");
       
        if(write_done)
        {
            flag=1;
            wake_up_interruptible(&wq);
        }

        hrtimer_forward_now(t,interval);
        return HRTIMER_RESTART;
}

/* Read: blocks until timer event */

static ssize_t my_read(struct file *file,char __user *buf,size_t len,loff_t *off)
{

        pr_info("Read: Waiting...\n");
        
       int remaining=strlen(msg)-*off;
          if(remaining<=0)
        {
                write_done=0;
                return 0;
        }
         int bytes_to_copy=min(len,remaining);

        wait_event_interruptible(wq,flag!=0);
        flag=0;
         
        if(copy_to_user(buf,msg+(*off),bytes_to_copy))
        {
                return -EFAULT;
        }
        pr_info("Read: done\n");

        (*off)+=bytes_to_copy;

        return bytes_to_copy;;
}

static ssize_t my_write(struct  file *file,const char __user *buf,size_t len,loff_t *off)
{
        pr_info("write initiated\n");
        int bytes_to_copy=min(len,(size_t)sizeof(msg)-1);

        if(copy_from_user(msg,buf,bytes_to_copy))
        {
                return -EFAULT;
        }
        msg[bytes_to_copy]='\0';

        printk(KERN_INFO "FROM USER %s\n",msg);

        *off=0;

        flag=1;
        write_done=1;
        wake_up_interruptible(&wq);
        pr_info("write done\n");

        return bytes_to_copy;

}

/*
   *called when user opens /dev/waitqueue_2
   */

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
        
        init_waitqueue_head(&wq);

        /* Register device */
        major=register_chrdev(0,DEVICE_NAME,&fops);
        printk(KERN_INFO "major number=%d\n",major);

        /* Setup timer (1 sec) */
        interval=ktime_set(1,0);

        hrtimer_init(&my_timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
        my_timer.function=timer_callback;

        hrtimer_start(&my_timer,interval,HRTIMER_MODE_REL);
        return 0;
}


/* Exit */

static void __exit my_exit(void)
{
        pr_info("Driver unloaded\n");

        hrtimer_cancel(&my_timer);
        unregister_chrdev(major,DEVICE_NAME);
}


module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Techdhaba");
MODULE_DESCRIPTION("waitqueue_2 waiting for user input");
