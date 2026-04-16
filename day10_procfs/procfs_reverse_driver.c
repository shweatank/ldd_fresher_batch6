#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/uaccess.h>

#define PROC_NAME "proc_demo"
#define BUF_SIZE 128

static char proc_buffer[BUF_SIZE]="Hello from procfs\n";

/* Read callback */
static ssize_t proc_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
pr_info("Inside proc read func\n");
return simple_read_from_buffer(user_buf, count, ppos, proc_buffer, strlen(proc_buffer));
}

/* Write callback */
static ssize_t proc_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
pr_info("Inside proc write func\n");
if(count > BUF_SIZE - 1)
return -EINVAL;

if(copy_from_user(proc_buffer, user_buf, count))
return -EFAULT;
for(int i=0;i<(count-1)/2;i++){
char temp=proc_buffer[i];
proc_buffer[i]=proc_buffer[count-i-1];
proc_buffer[count-i-1]=temp;
}
proc_buffer[count]='\0';
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
