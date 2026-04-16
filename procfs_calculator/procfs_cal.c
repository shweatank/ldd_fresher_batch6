#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

#define PROC_NAME "procfs_cal"

enum cal_op { ADD = 0, SUB, MUL, DIV};

struct calculator {
   int a;
   int b;
   enum cal_op op;
   int result;
};

static struct calculator cal;

//static int major;
static wait_queue_head_t wq;
static int flag = 0;

//work
static struct work_struct my_work;

// Work handler function
static void my_work_handler(struct work_struct *work)
{
        pr_info("Work queue : Handler started\n");

        switch(cal.op)
        {
                case 0: cal.result =  ( cal.a + cal.b );
                        break;
                case 1: cal.result =  ( cal.a - cal.b );
                        break;
                case 3: if(cal.b == 0) cal.result = 0;
                          cal.result =  (cal.a / cal.b);
			break;
                case 2: cal.result =  (cal.a * cal.b );
			break;
        }
	flag = 1;
        wake_up_interruptible(&wq);

        pr_info("Workqueue : Handler finished\n");
}



static ssize_t proc_read(struct file *file, char __user* user_buf, size_t count, loff_t *ppos)
{
	pr_info("Inside proc read func\n");
        pr_info("procfs_cal : waiting for completing the work\n");
        wait_event_interruptible(wq, flag != 0);
	
	if(copy_to_user(user_buf, &cal, sizeof(struct calculator)))
		 return -EFAULT;
	flag = 0;
	return sizeof( struct calculator );
}

static ssize_t proc_write(struct file *file, const char __user* user_buf, size_t count, loff_t *ppos)
{
	pr_info("Inside proc write func \n");
	if((copy_from_user( &cal, user_buf, sizeof(struct calculator))))
		return -EFAULT;
        
        pr_info("procfs_cal : got the request scheduling work a=%d - b=%d\n",cal.a,cal.b);
        schedule_work(&my_work);
	return sizeof(struct calculator);
}

static const struct proc_ops proc_file_ops = {
        .proc_read = proc_read,
	.proc_write = proc_write,
};

static int __init proc_demo_init(void)
{
	proc_create(PROC_NAME, 0666, NULL, &proc_file_ops);
	pr_info("proc_demo loaded\n");
        
	init_waitqueue_head(&wq);

	INIT_WORK(&my_work, my_work_handler);

	return 0;
}

static void __exit proc_demo_exit(void)
{
       remove_proc_entry(PROC_NAME, NULL);
       flush_work(&my_work);
       //unregister_chrdev(major, PROC_NAME);

       pr_info("proc_demo unloaded\n");
}

module_init(proc_demo_init);
module_exit(proc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Almas");

