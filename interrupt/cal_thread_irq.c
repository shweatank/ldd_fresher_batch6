#include<linux/module.h>
#include<linux/interrupt.h>
#include<linux/delay.h>
#include<linux/io.h>
#include<linux/kernel.h>
#include <linux/minmax.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/uaccess.h>


#define IRQ_NUM 1
#define KBD_DATA_PORT 0x60

static int a = 0,b = 0,res=0;

#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256

static int major_number;
static char kernel_buffer[BUF_SIZE];
static int buffer_size;

static int basic_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "basic_char: device opened\n");
	return 0;
}

static int basic_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "basic_char: device closed\n");
	return 0;
}

static ssize_t basic_read(struct file *file,char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy;

	if (*offset >= buffer_size)
		return 0;

	bytes_to_copy = min(count, (size_t)(buffer_size - *offset));

	if (copy_to_user(user_buffer,kernel_buffer + *offset,bytes_to_copy))
		return -EFAULT;

	*offset += bytes_to_copy;

	printk(KERN_INFO "basic_char: read %d bytes\n", bytes_to_copy);

	return bytes_to_copy;
}

static ssize_t basic_write(struct file *file,const char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy;

	bytes_to_copy = min(count, (size_t)BUF_SIZE);

	if (copy_from_user(kernel_buffer, user_buffer, bytes_to_copy))
		return -EFAULT;

	kernel_buffer[bytes_to_copy] = '\0';


	sscanf(kernel_buffer, "%d %d", &a, &b);
	
	fsleep(5);

	buffer_size = snprintf(kernel_buffer, BUF_SIZE, "Result = %d\n", res);


	printk(KERN_INFO "basic_char: result = %d\n", res);

	return bytes_to_copy;
}

static struct file_operations basic_fops = {
	.owner = THIS_MODULE,
	.open = basic_open,
	.read = basic_read,
	.write = basic_write,
	.release = basic_release
};


static irqreturn_t irq_top(int irq,void *dev_id)
{
return IRQ_WAKE_THREAD;
}
static irqreturn_t irq_thread(int irq,void *dev_id)
{
//pr_info("Threaded IRQ handler (can sleep)\n");
unsigned char scancode;

    //Read scancode from keyboard data port
    scancode = inb(KBD_DATA_PORT);

    printk(KERN_INFO "Keyboard IRQ : scancode = 0x%x\n",scancode);

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

static int __init irq_threaded_init(void)
{
major_number = register_chrdev(0, DEVICE_NAME, &basic_fops);

	if (major_number < 0) {
		printk(KERN_ERR "basic_char: failed to register device\n");
		return major_number;
	}

	printk(KERN_INFO "basic_char: loaded\n");
	printk(KERN_INFO "major number = %d\n", major_number);
	printk(KERN_INFO "mknod /dev/%s c %d 0\n", DEVICE_NAME, major_number);
return request_threaded_irq(IRQ_NUM,irq_top,irq_thread,IRQF_SHARED,"irq_threaded",(void *)irq_thread);
}
static void __exit irq_threaded_exit(void)
{
free_irq(IRQ_NUM,(void *)irq_thread);
unregister_chrdev(major_number, DEVICE_NAME);
printk(KERN_INFO "basic_char: unloaded\n");
}

module_init(irq_threaded_init);
module_exit(irq_threaded_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("GANGADHAR");
MODULE_DESCRIPTION("Simple Linux Kernel Interrupt using THREADS");

