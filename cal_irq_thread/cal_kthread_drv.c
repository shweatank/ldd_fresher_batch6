#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>


#define DEVICE_NAME "cal_kthread_drv"
#define INTERRUPT_NAME "cal_kthread_intrp"

#define KBD_IRQ 1
#define KBD_DATA_PORT 0x60

static int major;
unsigned char scancode;

static int calculator_op(int a,int b,int op)
{
        switch(op)
        {
                case 0: return (a+b);

                case 1: return (a-b);

                case 3: if(b == 0) return 0;
                          return (a/b);
                case 2: return (a*b);
        }
        return -112;
}

enum cal_op { ADD = 0, SUB, MUL, DIV};

struct calculator {
   int a;
   int b;
   enum cal_op op;
   int result;
};

static struct calculator cal_req = {.a = 5, .b = 5, .result = -1191, .op = 0};

static irqreturn_t irq_top(int irq, void *dev_id)
{
	scancode  = inb (KBD_DATA_PORT);
	if( scancode == 0x02 ){
           pr_info("Thread IRQ invoked by %c scancode\n",scancode);
           return IRQ_WAKE_THREAD;
	}
	return IRQ_NONE;
}

static irqreturn_t irq_thread(int irq, void *dev_id)
{
        pr_info("Thread IRQ handler Performing Calcution\n");
        
	int a = cal_req.a, b = cal_req.b , op = cal_req.op;
        switch(op)
        {
                case 0: cal_req.result =  a + b;
                        break;
                case 1: cal_req.result =  a - b;
                        break;
                case 3: if(b == 0) return 0;
                          cal_req.result = a / b;
			break;
                case 2: cal_req.result = a * b;
			break;
        }
	pr_info("Thread IRQ handler Performed Calcution Result = %d\n",cal_req.result);
        return IRQ_HANDLED;
}

static int my_open( struct inode *inode, struct file *file )
{
        printk( KERN_INFO "%s: device opened\n",DEVICE_NAME );
        return 0;
}

static int my_release( struct inode *inode, struct file *file )
{
        printk( KERN_INFO "%s : device closed", DEVICE_NAME );
        return 0;
}

static ssize_t my_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy = sizeof (struct calculator);

        if( copy_to_user( user_buffer , &cal_req , bytes_to_copy ))
                return -EFAULT;

        printk(KERN_INFO "%s: read %d bytes \n",DEVICE_NAME, bytes_to_copy);

        return bytes_to_copy;
}

static ssize_t my_write( struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy = sizeof(struct calculator);

        if(copy_from_user( &cal_req , user_buffer, bytes_to_copy ))
                return -EFAULT;
     
        printk(KERN_INFO "Recieved data : a = %d b = %d op = %d\n",cal_req.a, cal_req.b, cal_req.op);

        return bytes_to_copy;
}

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = my_open,
  .read = my_read,
  .write = my_write,
  .release = my_release,
};

static int __init irq_threaded_init(void)
{

	major = register_chrdev(0, DEVICE_NAME, &fops);
	printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME, major);
        return request_threaded_irq(KBD_IRQ, irq_top, irq_thread, IRQF_SHARED, "irq_threaded", (void *)irq_thread);
}

static void __exit irq_threaded_exit(void)
{
	unregister_chrdev( major, DEVICE_NAME);
        free_irq(KBD_IRQ,(void *)irq_thread);
}

module_init(irq_threaded_init);
module_exit(irq_threaded_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Almas");
