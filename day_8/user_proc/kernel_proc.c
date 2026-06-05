#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/fd.h>
#include <linux/init.h>   


static int major_number;
#define PROC_NAME "proc_demo"
#define BUF_SIZE 128
static char kernel_buffer[BUF_SIZE];
static int buffer_size;
static int val1;
static int val2;
static char op;
static int flag=0;

//static char proc_buffer[BUF_SIZE]="hello from procfs\n";

static int basic_open(struct inode *inode,struct file *file)
{

        printk(KERN_INFO "basic_char:device opened\n");
        return 0;

}

static int basic_release(struct inode *inode,struct file *file)
{
        printk(KERN_INFO "basic_char: device closed\n");
        return 0;

}


static ssize_t proc_read(struct file *file, char __user*user_buf , size_t count,loff_t*ppos)
{
        pr_info("inside proc read funciton\n");

	pr_info("Read : waiting..\n");
        wait_event_interruptible(wq, flag !=0);
	flag=0;
        return simple_read_from_buffer(user_buf,count,ppos,proc_buffer,strlen(proc_buffer));

}

static ssize_t proc_write(struct file*file,const char __user *user_buf,
        size_t count, loff_t *ppos)
{
        pr_info("inside proc write func \n");
        if(count > BUF_SIZE -1)
                return -EINVAL;

        if(copy_from_user(proc_buffer,user_buf,count))
                return -EFAULT;

        proc_buffer[count]='\0';
	sscanf(proc_buffer,"%d,%d,%c",&val1,&val2,&op);
	flag=1;
	
	
        return count;
}



static const struct proc_ops proc_file_ops={
        .proc_read=proc_read,
        .proc_write=proc_write,
};

static int __init proc_demo_init(void)
{
        proc_create(PROC_NAME,0666 , NULL,&proc_file_ops);
        pr_info("proc_demo loaded\n");
	        major_number = register_chrdev(0, DEVICE_NAME, &basic_fops);
        if(major_number < 0)
        {
                printk(KERN_ERR "basic_char: failed to register device\n");
                return major_number;

        }

        printk(KERN_INFO "basic_char:loaded\n");
        printk(KERN_INFO "basic_char: major number =%d\n",major_number);
        printk(KERN_INFO "create device node with :\n");
        printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME, major_number);

        //return 0;
	
	init_waitqueue_head(&wq);



        return 0;
}

static void __exit proc_demo_exit(void)
{
        remove_proc_entry(PROC_NAME, NULL);
        pr_info("proc_demo unloaded\n");
	unregister_chrdev(major_number,DEVICE_NAME);
        printk(KERN_INFO "basic_char: unloaded\n");

}

static struct file_operations basic_fops={
.owner = THIS_MODULE,
.open = proc_read,
.read = proc_write,
.write = basic_write,
.release = basic_release,
};



module_init(proc_demo_init);
module_exit(proc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple procfs driver");
MODULE_AUTHOR("pavan");

