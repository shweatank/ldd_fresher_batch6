#include<linux/module.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/io.h>
#include<linux/kernel.h>

#define KBD_IRQ 1
#define KBD_DATA_PORT 0x60

int a = 10;
int b = 20;
static int res;

static irqreturn_t keyboard_interrupt(int irq , void *dev_id)
{	
    unsigned char scancode;

    scancode = inb(KBD_DATA_PORT);

    if(scancode == 0x3B) // F1 -> ADD
    {
        res = a + b;
        printk(KERN_INFO "ADD Result = %d\n", res);
    }
    else if(scancode == 0x3C) // F2 -> SUB
    {
        res = a - b;
        printk(KERN_INFO "SUB Result = %d\n", res);
    }
    else if(scancode == 0x3D) // F3 -> MUL
    {
        res = a * b;
        printk(KERN_INFO "MUL Result = %d\n", res);
    }
    else if(scancode == 0x3E) // F4 -> DIV
    {
        if(b != 0)
        {
            res = a / b;
            printk(KERN_INFO "DIV Result = %d\n", res);
        }
        else
        {
            printk(KERN_INFO "DIV Error: divide by zero\n");
        }
    }

    return IRQ_HANDLED;
}    

static int __init kbd_driver_init(void)
{
    int result;

    result = request_irq(KBD_IRQ , keyboard_interrupt , IRQF_SHARED , "kbd_driver" , (void *)keyboard_interrupt);
    if(result)
    {
        printk(KERN_ERR "cannot register IRQ REQ %d", KBD_IRQ);
        return result;
    }

    printk(KERN_INFO "Keyboard Driver Loaded\n");
    return 0;
}

static void __exit kbd_driver_exit(void)
{
    free_irq(KBD_IRQ , (void *)keyboard_interrupt);
    printk(KERN_INFO "Keyboard Driver Unloaded\n");
}

module_init(kbd_driver_init);
module_exit(kbd_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GANGADHAR");
MODULE_DESCRIPTION("Simple Linux Kernel Keyboard Interrupt Driver");
