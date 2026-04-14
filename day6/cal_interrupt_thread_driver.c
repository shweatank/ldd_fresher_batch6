#include<linux/module.h>
#include<linux/interrupt.h>
#include<linux/delay.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>

#define IRQ_NUM 1
#define DEVICE_NAME "calc_irq"

int num1,num2;
char op;
int result;

static int major;
static struct class *cls;

static irqreturn_t irq_top(int irq,void *dev_id)
{
	return IRQ_WAKE_THREAD;
}
static irqreturn_t irq_thread(int irq,void *dev_id)
{
	pr_info("Threaded IRQ handler:Performing calculation\n");
	msleep(50);
	switch(op)
	{
		case '+':result=num1+num2;
			 break;
		case '-':result=num1-num2;
			 break;
		case '*':result=num1*num2;
			 break;
		case '/':if(num2!=0)
				 result=num1/num2;
			 else
				 pr_info("Divide by zero is not possible\n");
			 break;
		default:pr_info("Invalid operator\n");
	}
	pr_info("Result=%d\n",result);
	return IRQ_HANDLED;
}
static ssize_t dev_write(struct file *f,const char __user *buf,size_t len,loff_t *off)
{
	char kbuf[50];
	if(copy_from_user(kbuf,buf,len))
	{
		pr_err("copy_from_user failed\n");
		return -EFAULT;
	}
	sscanf(kbuf,"%d %d %c",&num1,&num2,&op);
	pr_info("Received :%d %d %c\n",num1,num2,op);
	irq_thread(IRQ_NUM,NULL);
	return len;
}
static ssize_t dev_read(struct file *f,char __user *buf,size_t len,loff_t *off)
{
	char kbuf[50];
	int l;
	l=sprintf(kbuf,"%d\n",result);
	if(copy_to_user(buf,kbuf,l))
	{
		pr_err("copy_to_user failed\n");
		return -EFAULT;
	}
	return l;
}
static struct file_operations fops={
	.owner=THIS_MODULE,
	.read=dev_read,
	.write=dev_write,
};
static int __init irq_threaded_init(void)
{
	int ret;
	major=register_chrdev(0,DEVICE_NAME,&fops);
	cls=class_create("calc_class");
	device_create(cls,NULL,MKDEV(major,0),NULL,DEVICE_NAME);
	ret=request_threaded_irq(IRQ_NUM,irq_top,irq_thread,IRQF_SHARED,"irq_threaded",(void *)irq_thread);
	if(ret)
	{
		pr_err("IRQ reuqest failed\n");
		return ret;
	}
	pr_info("Calculator IRQ driver loaded\n");
	return 0;
}
static void __exit irq_threaded_exit(void)
{
	free_irq(IRQ_NUM,(void *)irq_thread);
	device_destroy(cls,MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("Calculator IRQ driver unloaded\n");
}
module_init(irq_threaded_init);
module_exit(irq_threaded_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sarika");
MODULE_DESCRIPTION("Interrupt threaded irq");
