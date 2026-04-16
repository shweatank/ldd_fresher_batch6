/*Design a Linux device driver that simulates a virtual device where a user-space application writes a request (e.g., “generate data”) to /dev/mydevice.
  The driver must:
  Simulate an interrupt (using a timer or software trigger) after receiving the request
  Handle the simulated interrupt in an ISR and defer work using a tasklet or workqueue
  Generate and store data inside the driver
  Put the user process to sleep using a waitqueue when it calls read() before data is ready
  Wake up the process once data is prepared
  Use a kernel thread to periodically generate or monitor data in the background*/

#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/io.h>
#include<linux/uaccess.h>
#include<linux/jiffies.h>
#include<linux/workqueue.h>
#include<linux/delay.h>
#include<linux/fs.h>
#include<linux/wait.h>

#define TIMER_INTERVAL_MS 5000
#define DEVICE_NAME "MODULE"
#define BUF_SIZE 256
static int major_number;
static char kernel_buffer[BUF_SIZE]="hello world";
static int buffer_size;
static struct timer_list my_timer;
static struct work_struct my_work;
static wait_queue_head_t wq;
int flag=0;
int rand=0;
static void my_work_handler(struct work_struct *work)
{
	pr_info("Workqueue : handler started\n");

	if(rand%2==0)
	{
		strscpy(kernel_buffer,"hello world",BUF_SIZE);
	}
	else
		strscpy(kernel_buffer,"hello india",BUF_SIZE);
	rand=rand+1;
	pr_info("workqueue:handler finished\n");
	flag=1;
	wake_up_interruptible(&wq);

}
//timer callback function
static void my_timer_callback(struct timer_list *t)
{
	pr_info("timer interrupt occured\n");
	//restart the timer periadic behevior
	pr_info("workqueue module loaded\n");
	//schedule work
	pr_info("workqueue:scheduling  work\n");
	schedule_work(&my_work);

//	mod_timer(&my_timer,jiffies +msecs_to_jiffies(TIMER_INTERVAL_MS));
}

static ssize_t myread(struct file *file,char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy;
	/*
	   if offset is beyond data, return 0 (EOF)
	 */

	if (flag==0)
		wait_event_interruptible(wq,flag!=0);
	flag=0;
	buffer_size = strlen(kernel_buffer);
	if(*offset >= buffer_size)
		return 0;
	bytes_to_copy = min(count,(size_t)(buffer_size - *offset));

	/*
	   copy data from kernel space to user space
	 */
	if(copy_to_user(user_buffer,kernel_buffer + *offset,bytes_to_copy))
		return -EFAULT;

	*offset += bytes_to_copy;

	printk(KERN_INFO "module: sended data successfully\n");
	return bytes_to_copy;
}

static ssize_t mywrite(struct file *file,const char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy;
	char temp[BUF_SIZE];
	bytes_to_copy = min(count, (size_t)BUF_SIZE);
	/*
	   copy data from the user space to kernel space 
	 */

	if(copy_from_user(temp,user_buffer,bytes_to_copy))
		return -EFAULT;
	temp[bytes_to_copy-1]='\0';
	pr_info("Timer started\n");
	//start timer
	mod_timer(&my_timer,jiffies +msecs_to_jiffies(TIMER_INTERVAL_MS));
	printk(KERN_INFO "module: recevie request successfully\n");
	return bytes_to_copy;
}

static struct file_operations basic_fops = {
	.owner = THIS_MODULE,
	.read = myread,
	.write = mywrite,
};


static int __init mod_init(void)
{
	//initializing timer
	timer_setup(&my_timer,my_timer_callback,0);
	init_waitqueue_head(&wq);
	//initialize work

	INIT_WORK(&my_work,my_work_handler);
	major_number = register_chrdev(0,DEVICE_NAME,&basic_fops);
	if(major_number < 0)
	{
		printk(KERN_ERR "module: failed to register device\n");
		return major_number;
	}

	printk(KERN_INFO "module: loaded\n");
	printk(KERN_INFO "module: major  number = %d\n",major_number);
	printk(KERN_INFO "create device node with:\n");
	printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);
	return 0;




}


static void __exit mod_exit(void)
{

	pr_info("Workqueue module exiting\n");
	//ensure work is completed before exit
	flush_work(&my_work);
	pr_info("Workqueue completed\n");
del_timer_sync(&my_timer);
	unregister_chrdev(major_number,DEVICE_NAME);
	printk(KERN_INFO "module: unloaded\n");


}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PRUDHVI");
MODULE_DESCRIPTION("VIRTUAL DEVICE TASK GENERATING RANDOM NUMBERS");


