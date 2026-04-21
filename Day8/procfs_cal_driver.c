#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/uaccess.h>
#include<linux/init.h>
#include<linux/wait.h>
#include<linux/workqueue.h>
#define PROC_NAME "proc_demo"
#define BUF_SIZE 128

struct calculator {
    int num1;
    int num2;
    char opr;
    int result;
};
static char proc_buffer[BUF_SIZE];
static struct work_struct my_work;
static wait_queue_head_t wait_q;
static int ready=1;
struct calculator cal;
/*work handler function*/
static void my_work_handler(struct work_struct *work)
{
       // pr_info("Workqueue:Handler started\n");
        /*simulate some work (sleep allowed)*/
        switch(cal.opr)
	{
	  case '+' : cal.result=cal.num1 + cal.num2; break;
          case '-' : cal.result=cal.num1 - cal.num2; break;
          case '*' : cal.result=cal.num1 * cal.num2; break;
	  case '/' : cal.result=(cal.num2!=0) ? cal.num1 / cal.num2: 0; break;
	  default:  cal.result=0;
	}
       // pr_info("workqueue:Handler finished\n");
       snprintf(proc_buffer, BUF_SIZE,
             "%d %c %d = %d\n",
             cal.num1, cal.opr, cal.num2, cal.result);

    pr_info("RESULT READY: %s", proc_buffer);

       ready=1;
       wake_up_interruptible(&wait_q);
}

/*Read callback*/
static ssize_t proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{

	pr_info("Inside proc read func\n");
	return simple_read_from_buffer(buf,count,ppos,proc_buffer,strlen(proc_buffer));

}

/*write callback*/
static ssize_t proc_write(struct file *file, const char __user *buf, size_t count ,loff_t *ppos)
{ 
	if(count < sizeof(struct calculator)) return -EINVAL;
	if(copy_from_user(&cal,buf,sizeof(struct calculator)))
		return -EINVAL;
	ready=0;
	 schedule_work(&my_work);
	 wait_event_interruptible(wait_q,ready==1);
        
	return count;
}

static const struct proc_ops proc_file_ops = {
	.proc_write = proc_write,
	.proc_read = proc_read,
};

static int __init proc_demo_init(void)
{
	proc_create(PROC_NAME,0666,NULL,&proc_file_ops);
	INIT_WORK(&my_work,my_work_handler);
	init_waitqueue_head(&wait_q);
        /*schedule work */
        pr_info("Workqueue:Scheduling work\n");
       // schedule_work(&my_work);
	pr_info("proc_demo loaded\n");
	return 0;
}

static void __exit proc_demo_exit(void)
{
	remove_proc_entry(PROC_NAME,NULL);
	flush_work(&my_work);
	pr_info("proc demo unloaded\n");
}

module_init(proc_demo_init);
module_exit(proc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NANDINI");
MODULE_DESCRIPTION("SIMPLE PROCFS DRIVER");


