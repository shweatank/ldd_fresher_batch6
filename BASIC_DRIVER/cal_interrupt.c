/*Create a system where a user sets a number through sysfs. On interrupt, 
perform a calculation on that number in a workqueue and wake up any process waiting for the result.
*/
#include<linux/module.h>
#include<linux/kobject.h>
#include<linux/sysfs.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/wait.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/io.h>
#include <linux/workqueue.h>
#include <linux/delay.h>


#define KBD_DATA_PORT 0x60

#define KBD_IRQ 1

static struct kobject *demo_kobj;
static int num1,num2,res;
static char op;
static wait_queue_head_t wq;
static int flag = 0;


//static int irq_counter = 0;
static int dev_id;

static struct work_struct my_work;
static irqreturn_t irq_demo_isr(int irq, void *dev_id)
{
	unsigned char scancode;

	//Read scancode from keyboard data port
	scancode = inb(KBD_DATA_PORT);
	if(scancode == 0X3B)
	{
		printk(KERN_INFO "Keyboard IRQ : scancode = 0x%x\n",scancode);
		pr_info("Interrupt occured\n");
		pr_info("Workqueue: Scheduling work\n");

		schedule_work(&my_work);



	}  
	return IRQ_HANDLED;
}


static void my_work_handler(struct work_struct *work)
{
	pr_info("Calculating\n");
	switch(op)
	{
		case '+':
			res = num1+num2;
			break;
		case '-':
			res = num1-num2;
			break;
		case '*':
			res = num1*num2;
			break;
		case '/':
			if(num2 == 0)
				return ;
			res = num1/num2;
			break;
		default:
			pr_info("Invalid option\n");
			return;
	}

	flag = 1;
	wake_up_interruptible(&wq);

}



static ssize_t value_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{

	wait_event_interruptible(wq , flag != 0);

	

	flag = 0;	
	return sprintf(buf,"%d\n",res);
}
static ssize_t value_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{
	sscanf(buf,"%d %c %d",&num1,&op,&num2);
	return count;
}

static struct kobj_attribute value_attr = __ATTR(value,0664,value_show,value_store);

static int __init sysfs_demo_init(void)
{
	demo_kobj = kobject_create_and_add("sysfs_demo",kernel_kobj);
	if(!demo_kobj)
		return -ENOMEM;
	init_waitqueue_head(&wq);
	INIT_WORK(&my_work, my_work_handler);

	int result;

	printk(KERN_INFO "Keyboard driver loaded\n");

	//Request IRQ 1 (keyboard)
	result = request_irq(KBD_IRQ , irq_demo_isr , IRQF_SHARED , "kbd_driver" , &dev_id);
	if(result)
	{
		printk(KERN_ERR "cannot register IRQ REQ %d", KBD_IRQ);
		return result;
	}


	sysfs_create_file(demo_kobj,&value_attr.attr);
	pr_info("sysfs_demo loaded\n");
	return 0;
}

static void __exit sysfs_demo_exit(void)
{

	kobject_put(demo_kobj);
	pr_info("sysfs_demo unloaded\n");

	flush_work(&my_work);

	pr_info("Workqueue module unloaded\n");
	
	 free_irq(KBD_IRQ, &dev_id);



}

module_init(sysfs_demo_init);

module_exit(sysfs_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gangadhar");
MODULE_DESCRIPTION("sysfs create");

