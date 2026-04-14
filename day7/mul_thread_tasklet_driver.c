#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/kthread.h>
#include<linux/delay.h>

#define IRQ_NUM 1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sarika");
MODULE_DESCRIPTION("Interrupt +Tasklet (Linux 6.8 Correct API)+thread Example");

/*shared data*/
static int num1=10,num2=20;
static int result=0;

/*kernel thread*/
static struct task_struct *thread_st;

/*forward declaration -NEW signature */
static void tasklet_fn(struct tasklet_struct *t);

/*Tasklet object*/
DECLARE_TASKLET(my_tasklet,tasklet_fn);
/*=========================
 * Tasklet (Bottom Half)
 * ======================== */
static void tasklet_fn(struct tasklet_struct *t)
{
	result=num1*num2;
	pr_info("tasklet:multiplication done=%d\n",result);
}

static int thread_fn(void *data)
{
	while(!kthread_should_stop())
	{
		pr_info("Thread:Result=%d\n",result);
		ssleep(2);
	}
	return 0;
}
/*========================
 * IRQ Handler (Top Half)
 * ======================= */
static irqreturn_t keyboard_irq_handler(int irq,void *dev_id)
{
	pr_info("interrupt:num1=%d num2=%d\n",num1,num2);
	tasklet_schedule(&my_tasklet);
	return IRQ_HANDLED;
}
/*=========================
 * Module Init
 * ======================== */
static int __init tasklet_irq_init(void)
{
	int ret;
	ret=request_irq(IRQ_NUM,keyboard_irq_handler,IRQF_SHARED,"kbd_tasklet",(void *)keyboard_irq_handler);
	if(ret)
	{
		pr_err("Failed to request IRQ %d\n",IRQ_NUM);
		return ret;
	}
	thread_st=kthread_run(thread_fn,NULL,"my_thread");
	if(IS_ERR(thread_st))
	{
		free_irq(IRQ_NUM,(void *)(keyboard_irq_handler));
		pr_info("Thread creation failed\n");
		return PTR_ERR(thread_st);
	}
	pr_info("tasklet module loaded\n");
	return 0;
}
/*======================
 * Module Exit
 * ===================== */
static void __exit tasklet_irq_exit(void)
{
	tasklet_kill(&my_tasklet);
	free_irq(IRQ_NUM,(void *)keyboard_irq_handler);
	if(thread_st)
	{
		kthread_stop(thread_st);
	}
	pr_info("tasklet module unloaded\n");
}
module_init(tasklet_irq_init);
module_exit(tasklet_irq_exit);

