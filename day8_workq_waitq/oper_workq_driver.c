/* program to take user input and modify value in work_handler and print output  in kernel space */ 
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/workqueue.h>
#include<linux/delay.h>
#include<linux/fs.h>
#include<linux/uaccess.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)


static struct work_struct my_work;
static int major;
//static int kernel_value=0;
static int work_data=0;

/*ioctl handler*/
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
int user_value;

switch(cmd){
case IOCTL_SET_VALUE:
/* copy data from user */
if(copy_from_user(&user_value,(int __user*)arg, sizeof(int)))
return -EFAULT;

pr_info("Kernel: received %d from user\n",user_value);
work_data=user_value;
schedule_work(&my_work);
break;

default:
return -EINVAL;
}
return 0;
}

static struct file_operations fops={
.owner=THIS_MODULE,
.unlocked_ioctl = basic_ioctl,
};

/* Work handler function */
static void my_work_handler(struct work_struct *work)
{
pr_info("Workqueue: handler started\n");
work_data += 20;
pr_info("THe modified number : %d\n",work_data);

/* Simulate some work (sleep allowed ) */
msleep(2000);

pr_info("Workqueue handler finished\n");
}

/* MOdule init */
static int __init workq_init(void)
{
pr_info("Workqueue module loaded\n");
major=register_chrdev(0,DEVICE_NAME,&fops);
pr_info("basic_ioctl loaded, major=%d\n",major);
/* Initialize work */
INIT_WORK(&my_work,my_work_handler);

/* Schedule work */
pr_info("Workqueue: Scheduling work\n");
schedule_work(&my_work);

return 0;
}

/* Module exit */
static void __exit workq_exit(void)
{

pr_info("Workqueue module exiting\n");
/* Ensure work is completed before exit */
flush_work(&my_work);
pr_info("Workqueue module unloaded\n");
unregister_chrdev(major,DEVICE_NAME);
pr_info("basic_ioctl unloaded\n");

}
module_init(workq_init);
module_exit(workq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VAISHNAVI");
MODULE_DESCRIPTION("Simple Workqueue Example for Linux kernel 6.17");
