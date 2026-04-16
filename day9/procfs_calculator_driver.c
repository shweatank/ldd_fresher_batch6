#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/uaccess.h>
#include<linux/workqueue.h>
#include<linux/delay.h>
#include<linux/wait.h>

#define PROC_NAME "proc_demo"
#define BUF_SIZE 128

static int num1,num2;
static char op;
static int result=0;
static char proc_buffer[BUF_SIZE];

static struct work_struct my_work;
static wait_queue_head_t wq;
static int data_ready=0;

/*work handler function*/
static void my_work_handler(struct work_struct *work)
{
        pr_info("Workqueue:Handler started\n");
        switch(op)
	{
		case '+':result=num1+num2;
			 break;
		case '-':result=num1-num2;
			  break;
		case '*':result=num1*num2;
			 break;
		case '/':if(num2!=0)
				 result=num1/num2;
			 break;
		default:result=0;
	}
	snprintf(proc_buffer,BUF_SIZE,"Result=%d\n",result);
	data_ready=1;
	wake_up_interruptible(&wq);
        pr_info("workqueue:Handler finished\n");
}

/*Read callback*/
static ssize_t proc_read(struct file *file,char __user *user_buf,size_t count,loff_t *ppos)
{
	pr_info("Inside proc read func\n");
	wait_event_interruptible(wq,data_ready==1);
	data_ready=0;
	return simple_read_from_buffer(user_buf,count,ppos,proc_buffer,strlen(proc_buffer));
}

/*write callback*/
static ssize_t proc_write(struct file *file,const char __user *user_buf,size_t count,loff_t *ppos)
{
	char buf[BUF_SIZE];
	pr_info("Inside proc write func\n");
	if(count>BUF_SIZE-1)
		return -EINVAL;
	if(copy_from_user(buf,user_buf,count))
		return -EINVAL;
	proc_buffer[count]='\0';
	pr_info("Received:%s\n",buf);
	sscanf(buf,"%d %c %d",&num1,&op,&num2);
	pr_info("num1=%d op=%c num2=%d",num1,op,num2);
	data_ready=0;
	schedule_work(&my_work);
	pr_info("work scheduled\n");
	return count;
}
static const struct proc_ops proc_file_ops={
	.proc_read=proc_read,
	.proc_write=proc_write,
};
static int __init proc_demo_init(void)
{
	proc_create(PROC_NAME,0666,NULL,&proc_file_ops);
	INIT_WORK(&my_work,my_work_handler);
	init_waitqueue_head(&wq);
	pr_info("proc_demo loaded\n");
	return 0;
}
static void __exit proc_demo_exit(void)
{
	remove_proc_entry(PROC_NAME,NULL);
	flush_work(&my_work);
	pr_info("proc_demo unloaded\n");
}
module_init(proc_demo_init);
module_exit(proc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sarika");
MODULE_DESCRIPTION("proc_fs calculator driver");
