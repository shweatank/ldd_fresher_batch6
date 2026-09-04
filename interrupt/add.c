/*Problem 1: Addition Using Interrupt, Tasklet, and Thread
Write a Linux kernel module where:
Interrupt reads two numbers
Tasklet adds them
Kernel thread prints the result*/
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/io.h>
#include<linux/delay.h>
#include<linux/kthread.h>

#define IRQ_NUM 1

#define KEYBOARD_IRQ 1
MODULE_LICENSE("GPL");
MODULE_AUTHOR("GANGADHAR");
MODULE_DESCRIPTION("Simple Linux Kernel Keyboard Interrupt Driver");
#define KBD_DATA_PORT 0x60

static int num1 = 10,num2 = 20,res = 0;

static struct task_struct *my_thread;

//forward declaration _ new signature
static void keyboard_tasklet_fn(struct tasklet_struct *t);

static int thread_fn(void *data);
//tasklet object
DECLARE_TASKLET(keyboard_tasklet,keyboard_tasklet_fn);

//tasklet bottom half
static void keyboard_tasklet_fn(struct tasklet_struct *t)
{
	
	res = num1 + num2;
	//pr_info("TASKLET: BOTTOM HALF EXECUTED\n");

}

//IRQ HANDler top half
static irqreturn_t keyboard_irq_handler(int irq,void *dev_id)
{

	//pr_info("irq : keyboard interrupt occured\n");

	tasklet_schedule(&keyboard_tasklet);

	return IRQ_HANDLED;
}

int thread_fn(void *data)
{

ssleep(4);
printk(KERN_INFO "The res %d\n",res);

return 0;
}
static int __init tasklet_irq_init(void)
{
	int ret;
	ret = request_irq(KEYBOARD_IRQ,keyboard_irq_handler,IRQF_SHARED,"kbd_tasklet",(void *)keyboard_irq_handler);

	if(ret)
	{
		pr_err("Falied to request IRQ %d\n",KEYBOARD_IRQ);
		return ret;
	}
	my_thread = kthread_run(thread_fn,NULL,"my_thread");	

	
	return 0;
}

//module exit
static void __exit tasklet_irq_exit(void)
{
	tasklet_kill(&keyboard_tasklet);
	free_irq(KEYBOARD_IRQ,(void *)keyboard_irq_handler);
	
	if(my_thread)
		kthread_stop(my_thread);
	pr_info("tasklet module unloaded\n");
}


module_init(tasklet_irq_init);
module_exit(tasklet_irq_exit);
