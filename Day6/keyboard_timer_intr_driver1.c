#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
//#include <linux/tasklet.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define KEYBOARD_IRQ 1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nandini");
MODULE_DESCRIPTION("Keyboard IRQ + Tasklet (Linux 6.8 correct API)");

static int num1=10,num2=20;
static int result=0;
/*kernel thread*/
static struct task_struct *thread_st;

/*Forward declaration ~ NEW signature */
static void keyboard_tasklet_fn(struct tasklet_struct *t);

/*Tasklet object*/
DECLARE_TASKLET(keyboard_tasklet, keyboard_tasklet_fn);

/*=========================
 * Tasklet (Bottom Half)
 * ========================*/
static void keyboard_tasklet_fn(struct tasklet_struct *t)
{
        result = num1 - num2;
	pr_info("tasklet: Addition done = %d\n",result);
}
/* =======================
 * Thread
 * ===================*/
static int thread_fn(void *data)
{
	while(!kthread_should_stop())
	{
	 pr_info("Thread: Result = %d\n",result);
	 ssleep(2); //print every 2 sec
	}
  return 0;
}

/*==========================
 * IRQ Handler
 */
static irqreturn_t keyboard_irq_handler(int irq,void *dev_id)
{
  /*Simulating reading numbers*/
  num1++;
  num2++;

  pr_info("Interrupt: num1 = %d num2 = %d\n",num1,num2);
  tasklet_schedule(&keyboard_tasklet);
  return IRQ_HANDLED;
}

/*========================
 * Module init
 * ========================*/
static int __init tasklet_irq_init(void)
{
 int ret;
 ret= request_irq(KEYBOARD_IRQ, keyboard_irq_handler,IRQF_SHARED ,"kbd_tasklet", (void *)keyboard_irq_handler);
 if(ret)
 {
	 pr_err("Failed ;to request IRQ %d\n", KEYBOARD_IRQ);
	 return ret;
 }
  /*Create kernel thread*/
  thread_st = kthread_run(thread_fn, NULL,"my_thread");
  if(IS_ERR(thread_st))
  {
    free_irq(KEYBOARD_IRQ, keyboard_irq_handler);
    pr_err("Thread creation failed\n");
    return PTR_ERR(thread_st);
  }
 pr_info("tasklet module loaded\n");
 return 0;
}

/*==============================
 * Module exit
 * ===========================*/
static void __exit tasklet_irq_exit(void)
{
  tasklet_kill(&keyboard_tasklet);
  free_irq(KEYBOARD_IRQ, (void *)keyboard_irq_handler);
  if(thread_st)
    kthread_stop(thread_st);
  pr_info("tasklet module unloaded\n");
}

module_init(tasklet_irq_init);
module_exit(tasklet_irq_exit);
