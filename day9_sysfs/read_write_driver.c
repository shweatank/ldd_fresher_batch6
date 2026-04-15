/* driver to trigger interrupt when request is received from the user and as the interrrupt is ISR, it can not pefomr heavy work and it should be fast. So, call tasklet to generate 
 and store data to driver and wake up the process and then read() to user space */ 
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

#define DEVICE_NAME "waitq_basic"

static int major;
/* waitq */
static wait_queue_head_t wq;
static int flag=0;

static char data[100];

/* timer (simulated interrupt) */
static struct  hrtimer my_timer;
static ktime_t interval;

/* tasklet */
static struct tasklet_struct my_tasklet;

/* thread */
static struct task_struct *thread;

/* tasklet (bottom half) */
void tasklet_fn(unsigned long arg)
{
snprintf(data,sizeof(data),"Generated data : %lu\n",jiffies);
flag=1;
pr_info("Tasklet: data generated\n");
wake_up_interruptible(&wq);
}


/* Timer callback (ISR simulation) */
static enum hrtimer_restart timer_callback(struct hrtimer *t)
{
pr_info("ISR : Timer interrupt\n");
tasklet_schedule(&my_tasklet);
return HRTIMER_NORESTART;
}

/* write : trigger request */
static ssize_t my_write(struct file *file,const char __user *buf, size_t len, loff_t *off)
{
char kbuf[50];
copy_from_user(kbuf,buf,len);
pr_info("Write: %s\n",kbuf);
flag=0;

/* start timer (simulate interrupt after 2 secs ) */
interval=ktime_set(2,0);
hrtimer_start(&my_timer,interval,HRTIMER_MODE_REL);
return len;
}


/* REad: wait until data ready */
static ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
pr_info("Read: waiting...\n");
wait_event_interruptible(wq,flag!=0); //the process will be in sleep in wait queue till the flag is set to 1 by tasklet indicatng data generated. once flag=1  process will be waken up by the tasklet and resume read()
flag=0;
copy_to_user(buf,data,strlen(data));
pr_info("Read: done\n");
return strlen(data);
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
pr_info("THread: running in background\n");
ssleep(5);
}
return 0;
}


/* Init */
static int __init my_init(void){
pr_info("Driver loaded\n");
init_waitqueue_head(&wq);

/* REgister device */
major=register_chrdev(0,DEVICE_NAME,&fops);

/* timer init */
hrtimer_setup(&my_timer,timer_callback,CLOCK_MONOTONIC, HRTIMER_MODE_REL);
my_timer.function=timer_callback;

/*tasklet init */
tasklet_init(&my_tasklet,tasklet_fn,0);

/* kernel thread */
thread = kthread_run(thread_fn,NULL,"my_thread");
return 0;
}

/* Exit */
static void __exit my_exit(void)
{
pr_info("Driver unloaded\n");
hrtimer_cancel(&my_timer);
tasklet_kill(&my_tasklet);

if(thread)
kthread_stop(thread);

unregister_chrdev(major,DEVICE_NAME);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");

