#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#define KEYBOARD_IRQ 1
#define IRQ_NUM 1
MODULE_LICENSE("GPL");
MODULE_AUTHOR("pavani");
MODULE_DESCRIPTION("keyboard IRQ + Tasklet (Linux 6.8 correct API)");
static int num1=10,num2=20,res;



/*forward declaration -NEW signature */
static void keyboard_tasklet_fn(struct tasklet_struct *t);

/*tasklet object*/
DECLARE_TASKLET(keyboard_tasklet,keyboard_tasklet_fn);

static irqreturn_t irq_top(int irq,void *dev_id)
{
	return IRQ_WAKE_THREAD;
}

/*=================================
 * Tasklet (Bottom Half)
 * ================================*/
static void keyboard_tasklet_fn(struct tasklet_struct *t)
{
	pr_info("tasklet:bottom_half executed\n");
	pr_info("%d\n",num1);
	pr_info("%d\n",num2);
	if(num1<num2)
	{
		res=1;
	}
	else if(num1>num2)
	{
		res=0;
	}
	//pr_info("Add=%d\n",res);
}

/*================================
 * IRQ Handler(Top half)
 * ================================*/
static irqreturn_t keyboard_irq_handler(int irq,void *dev_id)
{
	pr_info("irq: keyboard interrupt occurred\n");
	tasklet_schedule(&keyboard_tasklet);
	num1++,num2--;
	return IRQ_HANDLED;
}

static irqreturn_t irq_thread(int irq,void *dev_id)
{
	//pr_info("Threaded IRQ handler (can sleep)\n");
	if(res==1)
	{
		pr_info("num2 is Greater\n");
	}
	else if(res==0)
	{
		pr_info("num1 is Greater\n");
	}
	msleep(50);
	return IRQ_HANDLED;
}

/*========================
 * Module Init
 * =======================*/
static int __init tasklet_irq_init(void)
{
	int ret;
	ret=request_irq(KEYBOARD_IRQ,keyboard_irq_handler,IRQF_SHARED,"kbd_tasklet",(void *)keyboard_irq_handler);
	if(ret)
	{
		pr_err("Failed to request IRQ %d\n",KEYBOARD_IRQ);
		return ret;
	}
	pr_info("tasklet module loaded\n");
	//return 0;
	return request_threaded_irq(IRQ_NUM,irq_top,irq_thread,IRQF_SHARED,"irq threaded",(void *)irq_thread);

}

/*=======================
 * Module Exit
 * ======================*/
static void __exit tasklet_irq_exit(void)
{
	tasklet_kill(&keyboard_tasklet);
	free_irq(KEYBOARD_IRQ,(void *)keyboard_irq_handler);
	free_irq(IRQ_NUM,(void *)irq_thread);
	pr_info("tasklet module unloaded\n");
}

module_init(tasklet_irq_init);
module_exit(tasklet_irq_exit);

