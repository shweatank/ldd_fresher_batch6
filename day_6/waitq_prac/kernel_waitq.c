#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#define DEVICE_NAME "waitq_basic"
#define BUF_SIZE 256


static int major;
static wait_queue_head_t wq;
static int flag=0;

static char kernel_buffer[BUF_SIZE];
static int buffer_size;


static struct hrtimer my_timer;
static ktime_t interval;

static int major_number;


static int val1;
static int val2;
static int result;



//timer callback

/*static enum hrtimer_restart timer_callback(struct hrtimer *t)
{
	pr_info("timer fired\n");
	flag=1;
	wake_up_interruptible(&wq);
	hrtimer_forward_now(t, interval);
	return HRTIMER_RESTART;
}*/


static int basic_open(struct inode *inode,struct file *file)
{

        printk(KERN_INFO "basic_char:device opened\n");
        return 0;

}

//read : blocks until timer event

static ssize_t my_read(struct file *file, char __user*buf,size_t len,loff_t *off)
{
	char msg[]="hello from kernel\n";
	pr_info("Read : waiting..\n");
	wait_event_interruptible(wq, flag !=0);
	flag=0;
	copy_to_user(buf,msg,sizeof(msg));
	pr_info("read : done\n");
	return sizeof(msg);
}

static ssize_t basic_write(struct file *file,const char __user *user_buffer,
                        size_t count,loff_t *offset)
{
        int bytes_to_copy;
        bytes_to_copy=min(count,(size_t)BUF_SIZE);
        //copy the data from user space to kernal space
        if(copy_from_user(kernel_buffer,user_buffer,bytes_to_copy))
                return -EFAULT;

        buffer_size=bytes_to_copy;
        sscanf(user_buffer,"%d,%d",&val1,&val2);
	flag = 1;

        printk(KERN_INFO "basic_char: wrote %d bytes\n",bytes_to_copy);
        return bytes_to_copy;
}



static struct file_operations fops=
{
	.owner = THIS_MODULE,
	.open = basic_open,
        .write = basic_write,	
	.read = my_read,
};

//init

static int __init my_init(void)
{
	pr_info("driver loaded\n");
	init_waitqueue_head(&wq);
	//register device
	major = register_chrdev(0,DEVICE_NAME, &fops);
	//setup timer (1sec)
//	interval= ktime_set(1,0);
//	hrtimer_setup(&my_timer,timer_callback,CLOCK_MONOTONIC, HRTIMER_MODE_REL);
//	my_timer.function=timer_callback;
//	hrtimer_start(&my_timer,interval,HRTIMER_MODE_REL);
	        major_number = register_chrdev(0, DEVICE_NAME, &fops);
        if(major_number < 0)
        {
                printk(KERN_ERR "basic_char: failed to register device\n");
                return major_number;

        }

        printk(KERN_INFO "basic_char:loaded\n");
        printk(KERN_INFO "basic_char: major number =%d\n",major_number);
        printk(KERN_INFO "create device node with :\n");
        printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME, major_number);

        return 0;


//	return 0;
}

//exit

static void __exit my_exit(void)
{
	pr_info("driver unloaded\n");

	hrtimer_cancel(&my_timer);
	unregister_chrdev(major,DEVICE_NAME);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pavan");
MODULE_DESCRIPTION("this program is about to know the how wait_queue is works");

