/* program to deincrement demo value through keyboard interrupt */
#include<linux/module.h>
#include<linux/kobject.h>
#include<linux/sysfs.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>

#define IRQ_NO 1//here, i am using keyboard interrupt

static struct kobject *demo_kobj;
static int demo_value;

/* INterrupt handler */
static irqreturn_t irq_handler(int irq, void *dev_id){
demo_value++; //incrementing when interrupt occurs
pr_info("Interrupt occured ! the value is incremented to %d\n",demo_value);
return IRQ_HANDLED;
}

/* show function */
static ssize_t value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
return sprintf(buf,"%d\n",demo_value);
}

/* store function */
static ssize_t value_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
sscanf(buf,"%d",&demo_value);
return count;
}

static struct kobj_attribute value_attr = __ATTR(value,0664,value_show,value_store);

static int __init sysfs_irq_demo_init(void)
{
int ret;
demo_kobj=kobject_create_and_add("sysfs_irq_demo",kernel_kobj);
if(!demo_kobj)
return -ENOMEM;

sysfs_create_file(demo_kobj,&value_attr.attr);

ret=request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "sysfs_irq_demo", &demo_value);
if(ret)
{
pr_err("Cannot register IRQ!\n");
return ret;
}
pr_info("sysfs_demo_loaded\n");
return 0;
}

static void __exit sysfs_irq_demo_exit(void)
{
free_irq(IRQ_NO, &demo_value);
kobject_put(demo_kobj);
pr_info("sysfs_demo unloaded\n");
}

module_init(sysfs_irq_demo_init);
module_exit(sysfs_irq_demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Keyboard interrupt + sysfs driver");
MODULE_AUTHOR("VAISHNAVI");


