#include<linux/module.h>
#include<linux/kobject.h>
#include<linux/sysfs.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/io.h>
#include<linux/kernel.h>



static struct kobject *demo_kobj;
static int num1 ,num2 ,res,count = 0;


#define KBD_IRQ 1
#define KBD_DATA_PORT 0x60


static irqreturn_t keyboard_interrupt(int irq , void *dev_id)
{	
	unsigned char scancode;

	//Read scancode from keyboard data port
	scancode = inb(KBD_DATA_PORT);

	if(scancode >= 0X02 && scancode <= 0X0B)
	{
		count++;
		if((count%2)==0)
			num2 = scancode-1;
		else
			num1 = scancode-1;
		
		res = num1+num2;
		//printk(KERN_INFO "Keyboard IRQ : scancode = 0x%x\n",scancode);
		printk(KERN_INFO "Result is %d : \n",res);
	}
	return IRQ_HANDLED;
}


static ssize_t value_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
	return sprintf(buf,"%d\n",res);
}
static ssize_t value_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{
//	sscanf(buf,"%d",&demo_value);
	return count;
}

static struct kobj_attribute value_attr = __ATTR(value,0664,value_show,value_store);

static int __init sysfs_demo_init(void)
{
	demo_kobj = kobject_create_and_add("sysfs_demo",kernel_kobj);
	if(!demo_kobj)
		return -ENOMEM;

	sysfs_create_file(demo_kobj,&value_attr.attr);
	pr_info("sysfs_demo loaded\n");

	int result;

	//    printk(KERN_INFO "Keyboard driver loaded\n");

	//Request IRQ 1 (keyboard)
	result = request_irq(KBD_IRQ , keyboard_interrupt , IRQF_SHARED , "kbd_driver" , (void *)keyboard_interrupt);
	if(result)
	{
		printk(KERN_ERR "cannot register IRQ REQ %d", KBD_IRQ);
		return result;
	}


	return 0;
}

static void __exit sysfs_demo_exit(void)
{
	kobject_put(demo_kobj);
	pr_info("sysfs_demo unloaded\n");
	free_irq(KBD_IRQ , (void *)keyboard_interrupt);
	printk(KERN_INFO "Keyboard Driver Unloaded\n");
}

module_init(sysfs_demo_init);

module_exit(sysfs_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gangadhar");
MODULE_DESCRIPTION("sysfs create");

