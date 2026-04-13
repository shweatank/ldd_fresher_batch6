#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define IRQ_NUM 1
#define DEVICE_NAME "calc_irq"

static int majornumber;
static struct class *cls;
char op;
int num1,num2,result;

static irqreturn_t irq_top(int irq, void *dev_id)
{
        return IRQ_WAKE_THREAD;
}

static irqreturn_t irq_thread(int irq, void *dev_id)
{
  pr_info("Threaded IRQ handler (can sleep)\n");
  msleep(50);

  switch(op)
  {
   case '+': result = num1 + num2; break;
   case '-': result = num1 - num2; break;
   case '*': result = num1 * num2; break;
   case '/': if(num2==0){
		     pr_info("Divide by zero is not possible\n");
		     result=0;
		     break;
	     }
	     result = num1 / num2; break;
   case '%': result = num1 % num2; break;
   default:
	     pr_info("Invalid operation\n");
	     
  }
  pr_info("Result = %d\n", result);
  return IRQ_HANDLED;
}

static ssize_t  dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
  char kbuf[50];
  if(copy_from_user(kbuf, buf, len)){
	  return -EFAULT;
  }
   kbuf[len]='\0';
  sscanf(kbuf," %d %d %c", &num1, &num2, &op);
  pr_info("Received: %d %d %c\n", num1, num2, op);
  irq_thread(IRQ_NUM,NULL);
  return len;
}

static ssize_t  dev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
  char kbuf[50];
  int l;
  l=sprintf(kbuf, " %d\n",result);
  if(copy_to_user(buf, kbuf, l))
	  return -EFAULT;
  
  return l;
}

static struct file_operations fops={
	.owner=THIS_MODULE,
	.read = dev_read,
	.write =dev_write,
};


static int __init irq_threaded_init(void)
{
	majornumber = register_chrdev(0, DEVICE_NAME, &fops);
	cls = class_create("calc_class");
	device_create(cls, NULL, MKDEV(majornumber,0), NULL, DEVICE_NAME);
	
       int ret=request_threaded_irq(IRQ_NUM, irq_top, irq_thread, IRQF_SHARED, "irq_threaded",(void *)irq_thread);
       if(ret)
	       return ret;
       pr_info("Calculator irq loaded\n");
       return 0;
}

static void __exit irq_threaded_exit(void)
{
        free_irq(IRQ_NUM, (void *)irq_thread);
	device_destroy(cls, MKDEV(majornumber, 0));
	class_destroy(cls);
	unregister_chrdev(majornumber,DEVICE_NAME);
	pr_info("Calculator irq unloaded\n");
}

module_init(irq_threaded_init);
module_exit(irq_threaded_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nandini");
MODULE_DESCRIPTION("Irq with threads");                                                                                                                                                                                       
