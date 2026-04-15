#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#define IRQ_NUM 1
#define KEYBOARD_IRQ 1
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("Keyboard  irq + tasklet (linux 6.8 correct api)");
int num1=10,num2=20,sum;
/* forword direction- new signature */
static void keyboard_tasklet_fn(struct tasklet_struct *t);

/*Tasklet object*/
DECLARE_TASKLET(keyboard_tasklet,keyboard_tasklet_fn);
/*
*Tasklet bottom half
*/
static void keyboard_tasklet_fn(struct tasklet_struct *t)
{
	sum=num1-num2;
	printk(KERN_ALERT"Sum in tasklet:%d\n",sum);
	pr_info("tasklet: bottom half\n");
}
/*
IRQ handler(top half)
*/
static irqreturn_t irq_top(int irq,void* dev_id)
{
	return IRQ_WAKE_THREAD;
}
static irqreturn_t keyboard_irq_handler(int irq,void *dev_id)
{
	num1+=10,num2+=10;
	pr_info("irq:keyboard interrupt occured\n");
tasklet_schedule(&keyboard_tasklet);
	printk(KERN_INFO"Sum in irq_handler %d\n",sum);
	return IRQ_HANDLED;
}
/*
Module init
*/
static irqreturn_t irq_thread(int id,void *dev_id)
{
	pr_info("Thread IRQ handler can sleep\n");
	
	num1+=10,num2+=20;
	tasklet_schedule(&keyboard_tasklet);
	return IRQ_HANDLED;
}
static int __init tasklet_irq_init(void )
{
	int ret;
	return request_threaded_irq(IRQ_NUM,irq_top,irq_thread,IRQF_SHARED,"irq_threaded",(void*)irq_thread);
	if(ret)
	{
		pr_err("Failed to request IRQ %d\n",KEYBOARD_IRQ);
		return ret;
	}
pr_info("Tasklet module loaded\n");
	return  0;
}
//Module Exit 
static void __exit tasklet_irq_exit(void)
{
	tasklet_kill(&keyboard_tasklet);
	//free_irq(KEYBOARD_IRQ,(void*)keyboard_irq_handler);
	free_irq(IRQ_NUM,(void*)irq_thread);	
	pr_info("tasklet module unloaded\n");
}
module_init(tasklet_irq_init);
module_exit(tasklet_irq_exit);




