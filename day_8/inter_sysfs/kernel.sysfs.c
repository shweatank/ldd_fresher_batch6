#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>



#define KBD_IRQ 1
#define KBD_DATA_PORT 0x60

static int val1;
static int val2;
static char op;

static irqreturn_t keyboard_interrupt(int irq,void *dev_id)

{
        unsigned char scancode;
        //read scan code from keyboard data port
        scancode=inb(KBD_DATA_PORT);
	if(scancode==10)
		sscanf(buf, "%d,%d,%c",&val1,&val2,&op);
		
       // printk(KERN_INFO "keyboard IRQ: scan code =0x%x\n",scancode);
        return IRQ_HANDLED;

}

static ssize_t value_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
        switch(op){
                case '+':
                        result=val1+val2;
                        break;
                case '-':
                        result=val1-val2;
                        break;
                case '*':
                        result=val1*val2;
        }
        //result=val1+val2;
        return sprintf(buf,"%d\n",result);
}


static ssize_t value_store(struct kobject *kobj,struct kobj_attribute *attr,
                const char *buf, size_t count)
{
       // sscanf(buf, "%d,%d,%c",&val1,&val2,&op);
        return count;
}





static int __init kbd_driver_init(void)
{
        int result;

        printk(KERN_INFO "keyboard Driver loaded\n");
        //request IRQ 1 (keyboard)
        result = request_irq(KBD_IRQ,keyboard_interrupt,IRQF_SHARED,"kbd_driver",
                        (void*)(keyboard_interrupt));
        if(result){
        printk(KERN_ERR "Cannot register IRQ 1\n");
        return result;
        }
	demo_kobj = kobject_create_and_add("sysfs_demo",kernel_kobj);
        if(!demo_kobj)
                return -ENOMEM;

        sysfs_create_file(demo_kobj,&value_attr.attr);
        pr_info("sysfs_demo loaded\n");
        return 0;

}

static void __exit kbd_driver_exit(void)
{
        free_irq(KBD_IRQ,(void *)(keyboard_interrupt));
        printk(KERN_INFO "keyboard Driver Unloaded\n");
	kobject_put(demo_kobj);
        pr_info("sysfs_demo unloaded\n");

}

module_init(kbd_driver_init);
module_exit(kbd_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pavan");
MODULE_DESCRIPTION("simple keyboard interrupt driver");

