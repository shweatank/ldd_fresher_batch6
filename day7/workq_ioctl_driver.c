#include <linux/module.h>
#include <linux/fs.h>
#include<linux/kernel.h>
#include <linux/uaccess.h>
#include<linux/init.h>
#include<linux/workqueue.h>
#include<linux/delay.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)

static int major;
static int work_data;
static struct work_struct my_work;
/*work handler function*/
static void my_work_handler(struct work_struct *work)
{
        pr_info("Workqueue:Handler started\n");
        int result=work_data+10;
	pr_info("WorkQueue :Input=%d\n",work_data);
	pr_info("WorkQueue :Result=%d\n",result);
        pr_info("workqueue:Handler finished\n");
}

/*ioctl handler */
static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        int user_value;

        switch(cmd)
        {
        case IOCTL_SET_VALUE:
                /*copy data from user */
                if(copy_from_user(&user_value, (int __user *)arg, sizeof(int)))
                        return -EFAULT;

                pr_info("Kernel: received %d from user\n", user_value);
		work_data=user_value;
		schedule_work(&my_work);
                break;        
	default:
                return -EINVAL;
        }
        return 0;
}

static struct file_operations fops = {
        .owner = THIS_MODULE,
        .unlocked_ioctl = basic_ioctl,
};

static int __init basic_init(void)
{
        major = register_chrdev(0, DEVICE_NAME, &fops);
	pr_info("Workqueue module loaded,major=%d\n",major);
        /*initialize work*/
        INIT_WORK(&my_work,my_work_handler);
        /*schedule work */
        pr_info("Workqueue:Scheduling work\n");
        schedule_work(&my_work);
	return 0;
}
static void __exit basic_exit(void)
{
        unregister_chrdev(major,DEVICE_NAME);
        pr_info("Workqueue module exiting\n");
        /*Ensure work is completed before exit*/
        flush_work(&my_work);
        pr_info("Workqueue module unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sarika");
MODULE_DESCRIPTION("workqueue linux kernel");
