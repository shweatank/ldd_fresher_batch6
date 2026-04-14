#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

/* ---------- Keyboard Definitions ---------- */
#define KBD_IRQ 1
#define KBD_DATA_PORT 0x60

/* ---------- IOCTL Definitions ---------- */
#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'

struct calc {
    int a;
    int b;
    char op[10];   // not used now (kept for compatibility)
    int result;
};

#define IOCTL_CALC _IOWR(IOCTL_MAGIC, 1, struct calc)

/* ---------- Global Variables ---------- */
static int major;
static char current_op = 'A';   // Default: ADD

/* ---------- Keyboard ISR ---------- */
static irqreturn_t keyboard_interrupt(int irq, void *dev_id)
{
    unsigned char scancode;

    scancode = inb(KBD_DATA_PORT);

    /* Ignore key release */
    if (scancode & 0x80)
        return IRQ_HANDLED;

    switch (scancode) {
        case 0x1E:   // A
            current_op = 'A';
            printk(KERN_INFO "Operation set to ADD\n");
            break;

        case 0x30:   // B
            current_op = 'B';
            printk(KERN_INFO "Operation set to SUB\n");
            break;

        case 0x2E:   // C
            current_op = 'C';
            printk(KERN_INFO "Operation set to MUL\n");
            break;

        case 0x20:   // D
            current_op = 'D';
            printk(KERN_INFO "Operation set to DIV\n");
            break;
    }

    return IRQ_HANDLED;
}

/* ---------- IOCTL Handler ---------- */
static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct calc c;

    if (cmd != IOCTL_CALC)
        return -EINVAL;

    if (copy_from_user(&c, (struct calc __user *)arg, sizeof(c)))
        return -EFAULT;

    /* Perform operation based on last key pressed */
    switch (current_op) {
        case 'A':
            c.result = c.a + c.b;
            printk(KERN_INFO "ADD selected\n");
            break;

        case 'B':
            c.result = c.a - c.b;
            printk(KERN_INFO "SUB selected\n");
            break;

        case 'C':
            c.result = c.a * c.b;
            printk(KERN_INFO "MUL selected\n");
            break;

        case 'D':
            if (c.b != 0)
                c.result = c.a / c.b;
            else {
                printk(KERN_INFO "Divide by zero\n");
                c.result = 0;
            }
            break;

        default:
            c.result = 0;
    }

    printk(KERN_INFO "Kernel: %d and %d => result = %d\n",
           c.a, c.b, c.result);

    if (copy_to_user((struct calc __user *)arg, &c, sizeof(c)))
        return -EFAULT;

    return 0;
}

/* ---------- File Operations ---------- */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = basic_ioctl,
};

/* ---------- Module Init ---------- */
static int __init basic_init(void)
{
    int ret;

    printk(KERN_INFO "Loading Keyboard + IOCTL Driver\n");

    /* Register character device */
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ERR "Failed to register char device\n");
        return major;
    }

    printk(KERN_INFO "Device registered with major = %d\n", major);
    printk(KERN_INFO "Create device using:\n");
    printk(KERN_INFO "mknod /dev/%s c %d 0\n", DEVICE_NAME, major);

    /* Register keyboard IRQ */
    ret = request_irq(KBD_IRQ, keyboard_interrupt,
                      IRQF_SHARED, "kbd_calc_driver",
                      (void *)keyboard_interrupt);

    if (ret) {
        printk(KERN_ERR "Cannot register IRQ 1\n");
        unregister_chrdev(major, DEVICE_NAME);
        return ret;
    }

    printk(KERN_INFO "Keyboard IRQ registered\n");

    return 0;
}

/* ---------- Module Exit ---------- */
static void __exit basic_exit(void)
{
    free_irq(KBD_IRQ, (void *)keyboard_interrupt);
    unregister_chrdev(major, DEVICE_NAME);

    printk(KERN_INFO "Driver unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rahul");
MODULE_DESCRIPTION("Keyboard + IOCTL Calculator Driver");
