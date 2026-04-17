#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>

static struct kobject *demo_kobj;
static int demo_value;
#define KBD_IRQ 1
#define KBD_DATA_PORT 0X60
static int kernel_add=0;
static int kernel_mul=0;
static int kernel_sub=0;
struct num
{
	int a,b;
};
char op[20] ;
// show function
static ssize_t value_show(struct kobject *kobj,
		struct kobj_attribute *attr,
		char *buf)
{
	return sprintf(buf, "%d\n", demo_value);
}
static irqreturn_t keyboard_interrupt(int irq,void *dev_id)
{
	unsigned char scancode;

	scancode =inb(KBD_DATA_PORT);


	if(scancode==0x9E)
	{
		printk(KERN_INFO"keyboard IRQ:SCAN CODE =0x%x \n",scancode);
		printk(KERN_INFO"Add=%d \n",kernel_add);
	}
	else if(scancode==0x9F)
	{
		printk(KERN_INFO"keyboard IRQ:SCAN CODE =0x%x \n",scancode);
		printk(KERN_INFO"sub=%d \n",kernel_sub);
	}
	else if(scancode==0xB2)
	{
		printk(KERN_INFO"keyboard IRQ:SCAN CODE =0x%x \n",scancode);
		printk(KERN_INFO"mul=%d \n",kernel_mul);
	}
	return IRQ_HANDLED;

}

// store function
static ssize_t value_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf,
		size_t count)
{
	sscanf(buf,"%d%d%s", &a,&b,op);
	kernel_add=a+b;
	kernel_sub=k.a-b;
	kernel_mul=k.a*b;
	return count;
}

static struct kobj_attribute value_attr =
__ATTR(value, 0664, value_show, value_store);

static int __init sysfs_demo_init(void)
{
	int result;
	printk(KERN_INFO"keyboard driver loaded\n");

	result =request_irq(KBD_IRQ,
			keyboard_interrupt,
			IRQF_SHARED,
			"kbd_driver",
			(void*)(keyboard_interrupt));
	if(result)
	{
		printk(KERN_ERR"connot register IRQ 1\n");
		return result;
	}

	demo_kobj = kobject_create_and_add("sysfs_demo", kernel_kobj);

	if (!demo_kobj)
		return -ENOMEM;

	if (sysfs_create_file(demo_kobj, &value_attr.attr)) {
		kobject_put(demo_kobj);
		return -ENOMEM;
	}

	pr_info("sysfs_demo loaded\n");
	return 0;
}

static void __exit sysfs_demo_exit(void)
{
	free_irq(KBD_IRQ,(void*)(keyboard_interrupt));
	printk(KERN_INFO"keboard brover unloadded\n");

	kobject_put(demo_kobj);
	pr_info("sysfs_demo unloaded\n");
}

module_init(sysfs_demo_init);
module_exit(sysfs_demo_exit);

MODULE_LICENSE("GPL");
`MODULE_AUTHOR("Prudhvi");
