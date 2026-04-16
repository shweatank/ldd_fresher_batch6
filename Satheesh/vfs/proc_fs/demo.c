#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/uaccess.h>
#include<linux/init.h>
#include<linux/wait.h>
#include<linux/kernel.h>
#include<linux/workqueue.h>

#define PROC_NAME "proc_demo"
#define BUF_SIZE 128
static char proc_buffer[BUF_SIZE] = "Hello from procs";
int flag=0;
/*	WaitQ	  */
static wait_queue_head_t wq;
/*	workqueue	  */
static struct work_struct my_work;
static void work_handler(struct work_struct*work)
{
	int num1,num2,res;
	char op[10];
	sscanf(proc_buffer,"%d %d %s",&num1,&num2,op);
	if(strncmp(op,"sum",3)==0)
	{
		res=num1+num2;
	}else if(strncmp(op,"sub",3)==0)
	{
		res=num1-num2;
	}else if(strncmp(op,"mul",3)==0)
	{
		res=num1*num2;
	}else if(strncmp(op,"div",3)==0)
	{
		res=num1/num2;
	}
	sprintf(proc_buffer,"%s : %d\n",op,res);
	flag=1;
	wake_up_interruptible(&wq);

}
// Read call back
static ssize_t proc_read(struct file* file,char __user* user_buf,size_t count,loff_t *ppos)
{
	wait_event_interruptible(wq,flag!=0);
	flag =0;
	pr_info("Inside proc read func\n");
	return simple_read_from_buffer(user_buf,count,ppos,proc_buffer,strlen(proc_buffer));
}
/*Write call back*/
static ssize_t proc_write(struct file*file,const char __user*user_buf,size_t count,loff_t *ppos)
{
	pr_info("Inside proc write func\n");
	if(count>BUF_SIZE-1)
		return -EINVAL;
	if(copy_from_user(proc_buffer,user_buf,count))
			return -EINVAL;
	proc_buffer[count]='\0';
	schedule_work(&my_work);
	return count;
}
static const struct proc_ops proc_file_ops={
		.proc_read=proc_read,
		.proc_write=proc_write,};

static int __init proc_demo_init(void){
	proc_create(PROC_NAME,0664,NULL,&proc_file_ops);
	pr_info("proc_demo loaded\n");
	INIT_WORK(&my_work,work_handler);
	init_waitqueue_head(&wq);
	return 0;
}
static void __exit proc_demo_exit(void)
{
	remove_proc_entry(PROC_NAME,NULL);
	pr_info("proc_demo unloaded\n");
}
module_init(proc_demo_init);
module_exit(proc_demo_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh Chintu");
MODULE_DESCRIPTION("Simple proc driver");

