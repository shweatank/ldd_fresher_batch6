#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>


struct st{
int val;
char buf[256];
int count;
};

#define DEVICE_NAME "ioctl_RW"
#define IOCTL_MAGIC 'B'
#define IOCTL_READ_WRITE _IOWR(IOCTL_MAGIC,1,struct st)
#define BUF_SIZE 256

static char kernel_buffer[BUF_SIZE];

static int major;

static int basic_open(struct inode *inode,struct file *file)
{
	printk(KERN_INFO "IOCTL_RW:device opened\n");
	return 0;
}

//called when user closes /dev/ioctl_rw

static int basic_release(struct inode *inode,struct file *file)
{
	printk(KERN_INFO"basic_char: device closed\n");
	return 0;
}

// cals

static long ioctl_RW(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct st user_value;
	switch(cmd){
		case IOCTL_READ_WRITE:
			if(copy_from_user(&user_value,(struct st __user*)arg,sizeof(struct st)))
				return -EFAULT;

	if(copy_to_user((struct st __user*)arg,&user_value,sizof(struct st)))
		return -EFAULT;
	
	}

}

static struct file_operations fops=
{
	.owner=THIS_MODULE,
	.unlocked_ioctl = ioctl_RW,
	.open = basic_open,
	.read = basic_read,
	.write = basic_write,
	.release = basic_release,
};

static int __int basic_init(void)
{
	major=register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("basic_ioctl loaded,major=%d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("basic_ioctl unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("pavan");
MODULE_DESCRIPTION("this driver helps to usage of both ioctl and read and write functions");
