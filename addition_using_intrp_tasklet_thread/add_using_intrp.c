#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/io.h>

#define KBD_IRQ 1

struct calculator {
   int a;
   int b;
   int result;
};
static wait_queue_head_t wq;
static struct task_struct *task;

struct calculator cal = {.a = 12, .b = 13};
static int result_ready = 0;
static int kthread_print(void *arg)
{
	while(!kthread_should_stop())
	{
		wait_event_interruptible(wq, result_ready != 0);
	        pr_info("kthread: a = %d + b = %d calculated value  = %d",cal.a, cal.b, cal.result);
		result_ready = 0;
	}
	pr_info("Kernel thread stopped\n");
	return 0;
}

static void keyboard_tasklet_fn(struct tasklet_struct *t);

DECLARE_TASKLET(keyboard_tasklet, keyboard_tasklet_fn);

static void keyboard_tasklet_fn(struct tasklet_struct *t)
{
	cal.result = cal.a + cal.b;
        pr_info("tasklet: bottom half added two numbers\n");
	result_ready = 1;
	wake_up_interruptible(&wq);
}


static irqreturn_t irq_top(int irq, void *dev_id)
{
	tasklet_schedule(&keyboard_tasklet);
	return IRQ_HANDLED;
}

static int __init my_init(void)
{
        int ret;

        ret = request_irq( KBD_IRQ,
                           irq_top,
                          IRQF_SHARED,
                         "kbd_tasklet" ,
                         (void *)irq_top
                        );
        if(ret){
          pr_err("Failed to request IRQ %d\n", KBD_IRQ);
          return ret;
        }
	init_waitqueue_head(&wq);
        task = kthread_run(kthread_print, NULL,"print thread");
        pr_info("tasklet module loaded\n");
        return 0;
}

static void __exit my_exit(void)
{
        tasklet_kill(&keyboard_tasklet);
        free_irq(KBD_IRQ, (void *)irq_top);
        if(task)
		kthread_stop(task);
        pr_info("tasklet module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Almas");
