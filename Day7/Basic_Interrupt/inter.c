#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>

#define DRIVER_NAME "irq_demo_driver"
#define IRQ_NUM 1  //x86 keyboard interrupt
static int irq_counter = 0;
/*
 *interrupt service routine (ISR)
 *this runs in interrupt context (top half)
 */
static irqreturn_t irq_demo_isr(int irq,void *dev_id){
  irq_counter++;
  pr_info("%s: Interupt received ! IRQ=%d Count=%d\n",DRIVER_NAME,irq,irq_counter);
  /*
   * IRQ HANLDER MEANS : This interrupt was meant for us
   */
  return IRQ_HANDLED;
}

static int __init irq_demo_init(void){
  int ret;
  pr_info("%s: Initializing\n",DRIVER_NAME);
  /*
   * request_irq arguments :
   * irqc-> IRQ number
   * handler ->ISR FUNCTION
   * flags -> IRQF_SHARED allows sharing
   * name -> visible in /proc/interrupts
   * dev_id -> unique identifier (must match free irq)
   */
  ret = request_irq(IRQ_NUM,irq_demo_isr,IRQF_SHARED,DRIVER_NAME,(void *)irq_demo_isr);
  if(ret){
    pr_info("%s: Failed to request IRQ %d\n",DRIVER_NAME,IRQ_NUM);
    return ret;
  }
  pr_info("%s: IRQ %d registered successfully\n",DRIVER_NAME,IRQ_NUM);
  return 0;
}

static void __exit irq_demo_exit(void){

  pr_info("%s: Cleaning Up\n",DRIVER_NAME);
  /*
   *free_irq must match: -same IRQ number
   -same dev_id pointer
   */
  free_irq(IRQ_NUM,(void *)irq_demo_isr);
  pr_info("%s: IRQ Freed\n",DRIVER_NAME);
}

module_init(irq_demo_init);
module_exit(irq_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rahul");
MODULE_DESCRIPTION("Simple Linux kernal IRQ HANDLING Example");

