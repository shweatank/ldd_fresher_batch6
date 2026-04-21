#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/uaccess.h>
#include<linux/init.h>

#define PROC_NAME "proc_demo"
#define BUF_SIZE 128

static char proc_buffer[BUF_SIZE] = "Hello from procfs\n";

/*Read callback*/
static ssize_t proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	pr_info("Inside proc read func\n");
	return simple_read_from_buffer(buf,count,ppos,proc_buffer,strlen(proc_buffer));

}

/*write callback*/
static ssize_t proc_write(struct file *file, const char __user *buf, size_t count ,loff_t *ppos)
{
	int l,r;
	char temp;
	pr_info("Inside proc write func\n");
	if(count>BUF_SIZE-1)
		return -EINVAL; 
	if(copy_from_user(proc_buffer,buf,count))
		return -EINVAL;
	proc_buffer[count]='\0';
	l=0,r=strlen(proc_buffer)-1;
	if(l<r)
	{
	 temp=proc_buffer[l];
	 proc_buffer[l]=proc_buffer[r];
	 proc_buffer[r]=temp;
	 l++;
	 r--;
	}
	return count;
}

static const struct proc_ops proc_file_ops = {
	.proc_read = proc_read,
	.proc_write = proc_write,
};

static int __init proc_demo_init(void)
{
	proc_create(PROC_NAME,0666,NULL,&proc_file_ops);
	pr_info("proc_demo loaded\n");
	return 0;
}

static void __exit proc_demo_exit(void)
{
	remove_proc_entry(PROC_NAME,NULL);
	pr_info("proc demo unloaded\n");
}

module_init(proc_demo_init);
module_exit(proc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NANDINI");
MODULE_DESCRIPTION("SIMPLE PROCFS DRIVER");


