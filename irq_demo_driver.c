#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>

#define DRIVER_NAME "irq_demo_driver"
#define IRQ_NUM 1 /* exanmple :keyboard IRQ on x86 */

static int irq_counter=0;

/*
   * interrupt service routine (ISR)
   * THIS runs in interrupt context (top helf)
   */

static irqreturn_t irq_demo_isr(int irq,void *dev_id)
{
        irq_counter++;
        pr_info("%s: Interrupt received : IRQ=%d Count=%d\n",DRIVER_NAME,irq,irq_counter);

        /* 
           *IRQ_HANDLED means:
           * This interrupt was meant for us
           */
        return IRQ_HANDLED;
}


static int __init irq_demo_init(void)
{
        int ret;

        pr_info("%s: Initialization\n",DRIVER_NAME);

        /* * request_irq arguments:
           *irq      -> IRQ number
           *handler  -> ISR function
           *flags    -> IRQF_SHARED allows sharing
           *name     -> visiblen /proc/interrupts
          *dev_id    -> unique identifier(must match free_irq)
         */ 

        ret=request_irq(IRQ_NUM,irq_demo_isr,IRQF_SHARED,DRIVER_NAME,(void *)irq_demo_isr);


        if (ret)
        {
                pr_err("%s:Failed to request IRQ %d\n",DRIVER_NAME,IRQ_NUM);
                return ret;
        }

        pr_info("%s: IRQ %d requested successfully\n",DRIVER_NAME,IRQ_NUM);
        return 0;
}

static void __exit irq_demo_exit(void)
{
        pr_info("%s: Cleaning up\n",DRIVER_NAME);

        /*
           *free_irq must match:
           * - same IRQ number
           * - dame dev_id pointer
           */

        free_irq(IRQ_NUM,(void *)irq_demo_isr);

        pr_info("%s: IRQ freed\n",DRIVER_NAME);
}


module_init(irq_demo_init);
module_exit(irq_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("Simple Linux Kernel IRQ Hndling Example");
