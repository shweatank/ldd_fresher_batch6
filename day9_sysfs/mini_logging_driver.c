#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/wait.h>
#include<linux/hrtimer.h>
#include<linux/ktime.h>
#include<linux/interrupt.h>
#include<linux/kthread.h>
#include<linux/delay.h>
#include<linux/workqueue.h>

#define DEVICE_NAME "log_device"
#define MAXLOGS 10
#define LOGSIZE 100

/* log buffer */
static char log_buffer[MAXLOGS][LOGSIZE];
int log_count=0;

static int major;
/* waitq */
static wait_queue_head_t wq;
static int flag=0;

/* timer (simulated interrupt) */
static struct  hrtimer my_timer;
static ktime_t interval;

/* work queue */
static struct work_struct log_work;

/* thread */
static struct task_struct *thread;

/* workqueue */
void work_fn(struct work_struct *work)
{
pr_info("Workqueue: Processing logs\n");

flag=1;
wake_up_interruptible(&wq);

}


/* Timer callback (ISR simulation) */
static enum hrtimer_restart timer_callback(struct hrtimer *t)
{
pr_info("ISR : triggered\n");
schedule_work(&log_work);
return HRTIMER_NORESTART;
}
/* write : trigger request */
static ssize_t my_write(struct file *file,const char __user *buf, size_t len, loff_t *off)
{
if(log_count>=MAXLOGS)
{
pr_info("buffer full\n");
return -ENOMEM;
}
len=min(len,(size_t)(LOGSIZE-1));
copy_from_user(log_buffer[log_count],buf,len);
log_buffer[log_count][len]='\0';

pr_info("Write: Stored log[%d]: %s\n",log_count,log_buffer[log_count]);
log_count++;

/* start timer (simulate interrupt after 2 secs ) */
interval=ktime_set(2,0);
hrtimer_start(&my_timer,interval,HRTIMER_MODE_REL);
return len;
}


/* REad: wait until data ready */
static ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
int i,pos=0;
char out[500];
pr_info("Read: waiting for logs...\n");
wait_event_interruptible(wq,flag!=0); //the process will be in sleep in wait queue till the flag is set to 1 by tasklet indicatng data generated. once flag=1  process will be waken up by the tasklet and resume read()
for(i=0;i<log_count;i++)
{
pos += snprintf(out+pos,sizeof(out)-pos,"%s\n",log_buffer[i]);
copy_to_user(buf,out,pos);
}
log_count=0;
flag=0;
pr_info("Read: logs sent\n");
return pos;
}

static struct file_operations fops = {
.owner=THIS_MODULE,
.read=my_read,
.write=my_write,
};

/* KERNEL THREAD */
static int thread_fn(void *arg)
{
while(!kthread_should_stop())
{
if(log_count>0)
{
pr_info("THread: flushing logs..\n");

}
ssleep(5);
}
return 0;
}


/* Init */
static int __init my_init(void){
pr_info("log Driver loaded\n");
init_waitqueue_head(&wq);

/* REgister device */
major=register_chrdev(0,DEVICE_NAME,&fops);

/* work queue init */
INIT_WORK(&log_work,work_fn);

/* timer init */
hrtimer_setup(&my_timer,timer_callback,CLOCK_MONOTONIC, HRTIMER_MODE_REL);
my_timer.function=timer_callback;

/* kernel thread */
thread = kthread_run(thread_fn,NULL,"my_thread");
return 0;
}

/* Exit */
static void __exit my_exit(void)
{
pr_info("Driver unloaded\n");
hrtimer_cancel(&my_timer);
cancel_work_sync(&log_work);

if(thread)
kthread_stop(thread);

unregister_chrdev(major,DEVICE_NAME);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
