#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/uaccess.h>

#define PROC_NAME "proc_name"
#define BUF_SIZE 128


static int res;
static char proc_buffer[BUF_SIZE];
static int num1;
static int num2;
static char opp[4];
static struct work_struct my_work;

static void my_work_handler(struct work_struct *work)
{
        pr_info("Workqueue : Handler Started\n");
	if(!strcmp(opp,"add"))
	{
		res=num1+num2;
		pr_info("Workqueue : Addition is %d\n",res);
	}
	else if(!strcmp(opp,"sub"))
	{
		res=num1-num2;
                pr_info("Workqueue : Subtraction is %d\n",res);
	}
	else if(!strcmp(opp,"mul"))
	{
		res=num1*num2;
                pr_info("Workqueue : Multiplication is %d\n",res);
	}
	else if(!strcmp(opp,"div"))
	{
		res=num1/num2;
                pr_info("Workqueue : Division is %d\n",res);
	}
	else
	{
		pr_info("Workqueue : Enter Correct Operation");
	}

        pr_info("Workqueue : Handler Finished\n");
        //ready=1;
}

//Read callback
static ssize_t proc_read(struct file *file,char __user *user_buf,size_t count,loff_t *ppos)
{
	char s[128];
	pr_info("Inside proc read func \n");
	snprintf(s,128,"%d\n",res);
	return simple_read_from_buffer(user_buf,count, ppos,s,strlen(s));
}

//write callback
static ssize_t proc_write(struct file *file, const char __user *user_buf, size_t count,loff_t *ppos)
{
	pr_info("Inside proc write func \n");
	if(count > BUF_SIZE -1)
		return -EINVAL;

	if(copy_from_user(proc_buffer, user_buf, count))
		return -EFAULT;

	proc_buffer[count]= '\0';

	sscanf(proc_buffer,"%d %d %s",&num1,&num2,opp);
	schedule_work(&my_work);
	return count;
}

static const struct proc_ops proc_file_ops = {
	.proc_read = proc_read,
	.proc_write = proc_write,
};

static int __init proc_demo_init(void)
{
	proc_create(PROC_NAME, 0666, NULL, &proc_file_ops);
	INIT_WORK(&my_work, my_work_handler);
	pr_info("proc demo loaded\n");
	return 0;
}

static void __exit proc_demo_exit(void)
{
	remove_proc_entry(PROC_NAME, NULL);
	flush_work(&my_work);
	pr_info("proc demo unloaded\n");
}

module_init(proc_demo_init);
module_exit(proc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ram");
MODULE_DESCRIPTION("Simple Example for Proc_fs driver  in Linux");

