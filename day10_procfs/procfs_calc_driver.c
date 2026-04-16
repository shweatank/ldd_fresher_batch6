#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/uaccess.h>

#define PROC_NAME "proc_demo"
#define BUF_SIZE 128

static char proc_buffer[BUF_SIZE];
static int result;

/* Read callback */
static ssize_t proc_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
int len;
char output[50];

pr_info("Inside proc read func\n");
memset(output,0,sizeof(output));

len=sprintf(output,"%d\n",result);
if(*ppos >= len)
return 0;

return simple_read_from_buffer(user_buf, count, ppos, output, len);
}

/* Write callback */
static ssize_t proc_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
int a,b;
char op;

pr_info("Inside proc write func\n");
if(count > BUF_SIZE - 1)
return -EINVAL;

if(copy_from_user(proc_buffer, user_buf, count))
return -EFAULT;

proc_buffer[count]='\0';

/* remove newline from fgets */
if(proc_buffer[count-1]=='\n')
proc_buffer[count-1]='\0';

/* parse input */
if(sscanf(proc_buffer,"%d %d %c",&a,&b,&op) != 3)//storing values in a,b, op from proc_buffer
{
pr_err("Invalid input format !\n");
return -EINVAL;
}

switch(op){
case '+': result=a+b;
	break;
case '-': result=a-b;
	break;
case 'x': result=a*b;
	break;
case '/': if(b!=0)
	result=a/b;
	else
	result=0;
	break;
default: result=-1;
}
pr_info("Calc : %d %c %d = %d\n",a,op,b,result);
return count;
}

static const struct proc_ops proc_file_ops = {
.proc_read=proc_read,
.proc_write=proc_write,
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
pr_info("proc_demo unloaded\n");
}

module_init(proc_demo_init);
module_exit(proc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple procfs driver");
MODULE_AUTHOR("VAISHNAVI");
