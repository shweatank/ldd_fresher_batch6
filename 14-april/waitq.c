#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/wait.h>

#define DEVICE_NAME "waitq_basic"
static int major;
static wait_queue_head_t wq;
static int flag=0;
#define BUF_SIZE 256
static char kernel_buffer[BUF_SIZE];
static int buffer_size;
static int basic_open(struct inode *inode,struct file * file)
{
        printk(KERN_INFO"waitq_basic: device opened\n");
        return 0;
}
//called when user closes /dev/basic_char
static int basic_release(struct inode *inode,struct file *file)
{
        printk(KERN_INFO"basic_char: device closed\n");
        return 0;
}
static ssize_t basic_write(struct file *file,const char __user *user_buffer,size_t count,loff_t *offset)
{
        int bytes_to_copy;
        bytes_to_copy=min(count,(size_t)BUF_SIZE);
        /*
         * copy data from user space to kernel space
         */
        if(copy_from_user(kernel_buffer,user_buffer,bytes_to_copy))
                return -EFAULT;
	flag=1;
        wake_up_interruptible(&wq);
        buffer_size=bytes_to_copy;
        printk(KERN_INFO"basic_char: wrote %d bytes\n",bytes_to_copy);
        printk(KERN_INFO"I am in write\n");
        return bytes_to_copy;
}
/*read: blocks until time event*/
static ssize_t my_read(struct file *file,char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy;
	pr_info("Read: waiting....\n");
	wait_event_interruptible(wq,flag!=0);
	flag=0;
        bytes_to_copy=min(count,(size_t)(buffer_size-*offset));
        /*
         * copy data from kernel space to user space
         */
        if(copy_to_user(user_buffer,kernel_buffer+*offset,bytes_to_copy))
                return -EFAULT;
        printk(KERN_INFO"basic_char: read %d bytes\n",bytes_to_copy);
	pr_info("Read: done\n");
	return bytes_to_copy;
}
static struct file_operations fops={
	.owner=THIS_MODULE,
	.open   = basic_open,
	.read=my_read,
	.write  = basic_write,
	.release=basic_release,
};
/*init*/
static int __init my_init(void)
{
	pr_info("Driver loaded\n");
	init_waitqueue_head(&wq);
	/*register device*/
	major=register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("Major number: %d\n",major);
	return 0;
}
/*exit*/
static void __exit my_exit(void)
{
	pr_info("Driver unloaded\n");
	unregister_chrdev(major,DEVICE_NAME);
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
