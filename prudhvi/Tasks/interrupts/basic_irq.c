#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/interrupt.h>


#define DRIVER_NAME "irq_demo_driver"
#define IRQ_NUM 1 

static int irq_count=0;

// interrurpt service rouitine Isr
//this rule runs in interrupt context (top half)


static irqreturn_t irq_demo_isr(int irq,void *dev_id)
{
	irq_count++;
	pr_info("%s:INTERRUPT RECEVIED :IRQ =%d count=%d\n",DRIVER_NAME,irq,irq_count);
	//IRQ_HANDLER MEANS

	return IRQ_HANDLED;

}
static int __init irq_demo_init(void)
{
	int ret;
	pr_info("%s:initalling\n",DRIVER_NAME);
	ret=request_irq(IRQ_NUM,
			irq_demo_isr,
			IRQF_SHARED,
			DRIVER_NAME,
			(void*)irq_demo_isr);


	if(ret)
	{
		pr_err("%s:failed to request IRQ %d\n",DRIVER_NAME,IRQ_NUM);
		return ret;

	}

	pr_err("%s:IRQ %d reqistered successfully\n",DRIVER_NAME,IRQ_NUM);
	return 0;
}


static void __exit irq_demo_exit(void)
{
	pr_info("%s:Cleaning ip\n",DRIVER_NAME);
	//free_irq must match
	//same irq number
	//same dev_id pointer

	free_irq(IRQ_NUM,(void*)irq_demo_isr);
	pr_info("%s:IRQ Freed\n",DRIVER_NAME);

}
module_init(irq_demo_init);
module_exit(irq_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PRUDHVI");
MODULE_DESCRIPTION("simple linux kernel IRQ Handling example\n");


