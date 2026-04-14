#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include<linux/delay.h>
#define IRQ_NUM 1
#define KEYBOARD_IRQ 1
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anjali");
MODULE_DESCRIPTION("keyboard IRQ + Tasklet (Linux 6.8 correct API)");
static int num1=10,num2=40;
static int result;
/*forward declaration -NEW signature */
static void keyboard_tasklet_fn(struct tasklet_struct *t);

/*tasklet object*/
DECLARE_TASKLET(keyboard_tasklet,keyboard_tasklet_fn);

/*=================================
 * Tasklet (Bottom Half)
 * ================================*/
static void keyboard_tasklet_fn(struct tasklet_struct *t)
{
	result=num1*num2;
	pr_info("tasklet:bottom_half executed\n");
}

/*================================
 * IRQ Handler(Top half)
 * ================================*/
static irqreturn_t keyboard_irq_handler(int irq,void *dev_id)
{
	num1++;
	num2++;
	pr_info("irq: keyboard interrupt occurred\n");
	tasklet_schedule(&keyboard_tasklet);
	return IRQ_HANDLED;
}
static irqreturn_t irq_top(int irq,void * dev_id)
{
	return IRQ_WAKE_THREAD;
}
static irqreturn_t irq_thread(int irq,void * dev_id)
{
	pr_info("multiplication result= %d\n",result);
	pr_info("Threaded IRQ handler (can sleep)\n");
	msleep(50);
	return IRQ_HANDLED;
}
static int __init irq_threaded_init(void)
{	
	int ret;
	ret=request_irq(KEYBOARD_IRQ,keyboard_irq_handler,IRQF_SHARED,"kbd_tasklet",(void *)keyboard_irq_handler);
	if(ret)
	{
		pr_err("Failed to request IRQ %d\n",KEYBOARD_IRQ);
		return ret;
	}
	pr_info("tasklet module loaded\n");
	return request_threaded_irq(IRQ_NUM,
				    irq_top,
				    irq_thread,
				    IRQF_SHARED,
				    "irq_threaded",
				    (void *)irq_thread);
}
static void __exit irq_threaded_exit(void)
{
	tasklet_kill(&keyboard_tasklet);
	free_irq(KEYBOARD_IRQ,(void *)keyboard_irq_handler);
	free_irq(IRQ_NUM,(void *)irq_thread);
	pr_info("modules unloaded\n");
}
module_init(irq_threaded_init);
module_exit(irq_threaded_exit);
