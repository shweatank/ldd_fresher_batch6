#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#define KBD_IRQ 1
#define KBD_DATA_PORT 0x60

static irqreturn_t keyboard_interrupt(int irq,void *dev_id)
{
	unsigned char scancode;

	//read scan code from keyboard data port
	scancode=inb(KBD_DATA_PORT);
	int a=10,b=20;
	if(scancode & 0x80)
	{
        	return IRQ_HANDLED;
	}
	printk(KERN_INFO "keyboard IRQ: scan code =0x%x\n",scancode);
	if(scancode==0x1E)
	{
		printk(KERN_INFO "Addition:%d\n",a+b);
	}
	else if(scancode==0x1F)
	{
		printk(KERN_INFO "Subtraction:%d\n",a-b);
	}
	else if(scancode==0x32)
	{
		printk(KERN_INFO "multiplication:%d\n",a*b);
	}
	else if(scancode==0x20)
	{
		printk(KERN_INFO "division:%d\n",a/b);
	}
	return IRQ_HANDLED;
}
static int __init kbd_driver_init(void)
{
	int result;

	printk(KERN_INFO "Keyboard driver loaded\n");

	//request IRQ 1 (keyboard)
	result=request_irq(KBD_IRQ,keyboard_interrupt,IRQF_SHARED,"kbd driver",(void *)(keyboard_interrupt));
	if(result)
	{
		printk(KERN_ERR "cannot register IRQ 1\n");
		return result;
	}
	return 0;
}

static void __exit kbd_driver_exit(void)
{
	free_irq(KBD_IRQ,(void *)(keyboard_interrupt));
	printk(KERN_INFO "keyboard driver unloaded\n");
}

module_init(kbd_driver_init);
module_exit(kbd_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pavani");
MODULE_DESCRIPTION("simple keyboard interrupt handler");
