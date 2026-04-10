#include<linux/module.h>
#include<linux/interrupt.h>
#include<linux/delay.h>

#define IRQ_NUM 1

static irqreturn_t irq_top(int irq,void *dev_id)
{
	return IRQ_WAKE_THREAD;
}

static irqreturn_t irq_thread(int irq,void *dev_id)
{
	pr_info("threaded IRQ handler (can sleep)\n");
	msleep(50);
	return IRQ_HANDLED;
}

static int __init irq_thread_init(void)
{
	return request_threaded_irq(IRQ_NUM,
			irq_top,
			irq_thread,
			IRQF_SHARED,
			"irq_threaded",
			(void*)irq_thread);
}

static void __exit irq_thread_exit(void)
{
	free_irq(IRQ_NUM, (void*)irq_thread);
}

module_init(irq_thread_init);
module_exit(irq_thread_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("prudhvi");

MODULE_DESCRIPTION("interrupt using thread");
