#include <linux/module.h>   // Core kernel modules
#include <linux/kernel.h>   // Required for printk
#include <linux/init.h>     // Required for __init and __exit macros
#include <linux/fs.h>       // Register_chrdev, file_operations
#include <linux/uaccess.h>  // Copy_to_user, copy_from_user
#include <linux/interrupt.h>
#include <linux/io.h>	

#define DEVICE_NAME "scancode_cal_driver"

#define INTERRUPT_NAME "cal_scancode_driver"

#define KBD_IRQ 1
#define KBD_DATA_PORT 0x60

unsigned char scancode;

static int major_number;
struct calculator cal;

// Called when user opens /dev/basic_char

static int basic_open( struct inode *inode, struct file *file )
{
	printk( KERN_INFO "scancode_cal: device opened\n" );
	return 0;
}


// Called when user closes /dev/basic_char
static int basic_release( struct inode *inode, struct file *file )
{
	printk( KERN_INFO "scancode_cal: device closed" );
	return 0;
}

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
   int result;
};

static irqreturn_t keyboard_interrupt(int irq, void *dev_id)
{
        scancode  = inb (KBD_DATA_PORT);
        if(scancode == 0x02)
	{
                cal.result  = calculator_op(cal.a,cal.b,0);
		printk(KERN_INFO "Performing add result = %d\n",cal.result);
	}
        if(scancode == 0x03)
          cal.result  = calculator_op(cal.a,cal.b,1);
        if(scancode == 0x04)
          cal.result  = calculator_op(cal.a,cal.b,2);
        if(scancode == 0x05)
          cal.result  = calculator_op(cal.a,cal.b,3);

        printk(KERN_INFO "Keyboard IRQ: Scan Code = 0x%x\n",scancode);
        return IRQ_HANDLED;
}

// Called when user reads /dev/basic_char
static ssize_t basic_read(struct file *file,char __user *user_buffer,size_t count,loff_t *offset)
{
        int bytes_to_copy = sizeof( struct calculator );
        if( copy_to_user(user_buffer ,&cal , bytes_to_copy ))
                return -EFAULT;

        printk(KERN_INFO "cal_char: read %d bytes \n",bytes_to_copy);

        return bytes_to_copy;
}

static ssize_t basic_write(struct file *file,const char __user *user_buffer,size_t count, loff_t *offset)
{
	int bytes_to_copy = sizeof(struct calculator);	
        
        if(copy_from_user(&cal,user_buffer,bytes_to_copy))
		return -EFAULT;

	printk(KERN_INFO "Recieved data : a:%d b:%d\n",cal.a,cal.b);

	printk( KERN_INFO "cal_char: wrote %d bytes\n", bytes_to_copy );

	return bytes_to_copy;
}


/*
 *File operations structure
 *This connects system calls to driver functions
*/

static struct file_operations basic_fops = {
  .owner = THIS_MODULE,
  .open = basic_open,
  .read = basic_read,
  .write = basic_write,
  .release = basic_release,
};

/*
 * Module initailization 
*/

static int __init basic_char_init(void)
{
	//register character device
	//0 -> dynamic major number
	
	major_number = register_chrdev(0,DEVICE_NAME,&basic_fops);

	if(major_number < 0)
	{
		printk(KERN_INFO "cal_char: failed to register device\n");
		return major_number;
	}
        printk(KERN_INFO "cal_char : module loaded\n");
	printk(KERN_INFO "cal_char: major number = %d\n",major_number);
	printk(KERN_INFO "Created device node with : \n");
	printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);

        //interrupt
	
	int result;

        printk(KERN_INFO " Keyboard Driver Loaded\n");

        //request IRQ 1 (Keyboard)
        result   = request_irq ( KBD_IRQ,
                                 keyboard_interrupt,
                                 IRQF_SHARED,
                                 "kbd_driver",
                                 (void *)(keyboard_interrupt));
        if(result)
        {
                printk(KERN_ERR "Cannot register IRQ 1\n");
                return result;
        }


        return 0;  //Returning 0 means successful load
}

//module cleanup
//

static void __exit basic_char_exit(void)
{
	unregister_chrdev(major_number,DEVICE_NAME);
        printk(KERN_INFO "cal_char: unloaded\n");


        free_irq(KBD_IRQ, (void *)(keyboard_interrupt));
        printk(KERN_INFO "Keyboard Driver Unloaded\n");

}


//kernel module macros

module_init(basic_char_init);
module_exit(basic_char_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Almas");
MODULE_DESCRIPTION("Most basic linux kernel module for education");


