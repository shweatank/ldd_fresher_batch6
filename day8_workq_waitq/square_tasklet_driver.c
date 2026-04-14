/* program to simulate value through interrupt, find square in tasklet and print result through thread*/ 
#include<linux/module.h>
#include<linux/interrupt.h>
#include<linux/kthread.h>
#include<linux/wait.h>
#include<linux/delay.h>

#define IRQ_NO 1 //keyboard interrupt

static int a=0;
static int result;

static struct tasklet_struct my_tasklet;
static struct task_struct *thread;
static wait_queue_head_t wq;
static int flag=0;

/* kernel thread */
static int thread_fn(void *data)
{
while(!kthread_should_stop())
{
wait_event_interruptible(wq,flag==1);
printk("Square of %d=%d\n",a,result);
flag=0;
}
return 0;
}

/* tasklet */
void tasklet_fn(unsigned long data)
{
result=a*a; 
flag=1;
wake_up_interruptible(&wq);
}

/* interrupt */
irqreturn_t irq_handler(int irq, void *dev_id)
{
printk("Interrupt occured\n");

//input simulation
a=10;

tasklet_schedule(&my_tasklet);
return IRQ_HANDLED;
}

static int __init my_init(void)
{
init_waitqueue_head(&wq);
tasklet_init(&my_tasklet,tasklet_fn,0);
thread=kthread_run(thread_fn,NULL,"my_thread");
if(request_irq(IRQ_NO, irq_handler,IRQF_SHARED,"my_irq",&irq_handler))
{
printk("Cannot register IRQ\n");
return -1;
}
printk("Module loaded\n");
return 0;
}

static void __exit my_exit(void)
{
free_irq(IRQ_NO,&irq_handler);
tasklet_kill(&my_tasklet);
if(thread)
kthread_stop(thread);
printk("Module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
