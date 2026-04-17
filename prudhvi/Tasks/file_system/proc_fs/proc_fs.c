#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/uaccess.h>

#define PROC_NAME "proc_demo"
#define BUF_SIZE 128

static char proc_buffer[BUF_SIZE] ;

//read callback

static ssize_t proc_read(struct file *file,char __user *user_buf,size_t count,loff_t *ppos)
{
	pr_info("inside proc read func\n");
	return simple_read_from_buffer(user_buf,count,ppos,proc_buffer,strlen(proc_buffer));


}
//write callback
static ssize_t proc_write(struct file *file ,const char __user *user_buf,size_t count,loff_t *ppos)
{
	pr_info("inside proc write func\n");
	if(count >BUF_SIZE-1)
		return -EFAULT;
	if(copy_from_user(proc_buffer, user_buf, count))
	{
		return -EFAULT;
	}
	proc_buffer[count]='\0';
	int len=strlen(proc_buffer);
	int i,j;
	for(i=0,j=len-1-1;i<j;i++,j--)
	{
		char t=proc_buffer[i];
		proc_buffer[i]=proc_buffer[j];
		proc_buffer[j]=t;
	}
	return count;

}
static const struct proc_ops proc_file_op ={
	.proc_read =proc_read,
	.proc_write =proc_write,
};
static int __init proc_demo_init(void)
{
	proc_create(PROC_NAME,0666,NULL,&proc_file_op);
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
MODULE_AUTHOR("PRUDHVI");
MODULE_DESCRIPTION("simple procfs driver.h");
