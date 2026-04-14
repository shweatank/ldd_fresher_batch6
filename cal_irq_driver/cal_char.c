#include <linux/module.h>   // Core kernel modules
#include <linux/kernel.h>   // Required for printk
#include <linux/init.h>     // Required for __init and __exit macros
#include <linux/fs.h>       // Register_chrdev, file_operations
#include <linux/uaccess.h>  // Copy_to_user, copy_from_user
#include <linux/interrupt.h>
	
#define DEVICE_NAME "cal_char"

#define INTERRUPT_NAME "irq_demo_driver"
#define IRQ_NUM 1 
//Example : keyboard IRQ on x86

static int irq_counter = 0;

//interrupt service routine ISR
//This runs interrupt context (top half)

static irqreturn_t irq_demo_isr(int irq, void *dev_id)
{
        irq_counter++;

        pr_info("%s : Interrupt received! IRQ + %d Count = %d\n", INTERRUPT_NAME, irq, irq_counter);

        //IRQ_HANDLED means:
        //this is interrupt was meant for us

        return IRQ_HANDLED;
}


static int major_number;
static long long int result = -111; 
struct calculator cal;

// Called when user opens /dev/basic_char

static int basic_open( struct inode *inode, struct file *file )
{
	printk( KERN_INFO "cal_char: device opened\n" );
	return 0;
}


// Called when user closes /dev/basic_char

static int basic_release( struct inode *inode, struct file *file )
{
	printk( KERN_INFO "cal_char: device closed" );
	return 0;
}



// Called when user reads /dev/basic_char

static ssize_t basic_read(struct file *file,char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy = sizeof(long long int);
        if( copy_to_user(user_buffer ,&result , bytes_to_copy ))
		return -EFAULT;

	printk(KERN_INFO "cal_char: read %d bytes \n",bytes_to_copy);

	return bytes_to_copy;
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

enum cal_op {ADD=0,SUB,MUL,DIV};

struct calculator {
   int a;
   int b;
   enum cal_op op;
};

static ssize_t basic_write(struct file *file,const char __user *user_buffer,size_t count, loff_t *offset)
{
	int bytes_to_copy = sizeof(struct calculator);	
        
        if(copy_from_user(&cal,user_buffer,bytes_to_copy))
		return -EFAULT;

	printk(KERN_INFO "Recieved data : a:%d b:%d op=%d ",cal.a,cal.b,cal.op);
	result  = calculator_op(cal.a,cal.b,(int)cal.op);
        
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


	//interrupt init
	int ret;

        pr_info("%s : Initailization \n",INTERRUPT_NAME);
        /*
         * request_irq arguments
         * irq     -> IRQ number
         * handler  -> ISR functon
         * flags    -> IRQF_SHARED allows sharing
         * name     -> visible in /proc/interrupts
         * dev_id   -> unique identifier ( must match free_irq)
         */

         ret = request_irq( IRQ_NUM, irq_demo_isr, IRQF_SHARED, INTERRUPT_NAME, (void*)irq_demo_isr);
         if(ret)
         {
                 pr_err("%s : Failed to request IRQ %d\n", INTERRUPT_NAME, IRQ_NUM);
         }

         pr_info("%s : IRQ %d registered sucessfully\n", INTERRUPT_NAME, IRQ_NUM);

        return 0;  //Returning 0 means successful load
}

//module cleanup
//

static void __exit basic_char_exit(void)
{
	unregister_chrdev(major_number,DEVICE_NAME);
        printk(KERN_INFO "cal_char: unloaded\n");

	//interrupt exit
	 pr_info("%s: cleaning up\n", INTERRUPT_NAME);
        /*
         * free_irq must match
         * - smae IRQ number
         * - same dev_id pointer
         * */
        free_irq(IRQ_NUM, (void *)irq_demo_isr);
        pr_info("%s: IRQ freed\n", INTERRUPT_NAME);

}


//kernel module macros

module_init(basic_char_init);
module_exit(basic_char_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Almas");
MODULE_DESCRIPTION("Most basic linux kernel module for education");


