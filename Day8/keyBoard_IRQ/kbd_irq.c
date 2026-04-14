#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>

#define KEYBOARD_IRQ 1
MODULE_LICENSE("GPL");
MODULE_AUTHOR("RAHUL");
MODULE_DESCRIPTION("Keyboard IRQ + Tasklet (Linux 6.8 correct API)");

/*forward declararion - NEW signature */
static void keyboard_tasklet_fn(struct tasklet_struct *t);

/*Tasklet object*/
DECLARE_TASKLET(keyboard_tasklet,keyboard_tasklet_fn);

/*=========================*/
//tasklet (BOTTOM HALF)
/*=========================*/
static void keyboard_tasklet_fn(struct tasklet_struct *t){
  pr_info("Tasklet: bottom half executed\n");
}

/*=========================*/
// IRQ Handler (Top HALF)
/*=========================*/

static irqreturn_t keyboard_irq_handler(int irq,void *dev_id){
  pr_info("irq: keyboard interrupt occurred\n");

  tasklet_schedule(&keyboard_tasklet);
  return IRQ_HANDLED;
}
/*=========================*/
// Module Init
/*=========================*/

static int __init tasklet_irq_init(void){
  int ret;
  ret = request_irq(KEYBOARD_IRQ,keyboard_irq_handler,IRQF_SHARED,"kbd_tasklet",(void *)keyboard_irq_handler);
  if(ret){
    pr_info("Failed to request IRQ %d\n",KEYBOARD_IRQ);
    return ret;
  }
  pr_info("tasklet module loaded\n");
  return 0;
}

/*=========================*/
// Module Exit
/*=========================*/

static void __exit tasklet_irq_exit(void){
   tasklet_kill(&keyboard_tasklet);
   free_irq(KEYBOARD_IRQ,(void *)keyboard_irq_handler);
   pr_info("tasklet module loaded\n");
}
module_init(tasklet_irq_init);
module_exit(tasklet_irq_exit);

