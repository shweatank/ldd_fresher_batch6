#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "running_sum"

static int running_sum = 0;

static int major;
static char buffer[128];
static char result[32];
static int bytes_to_copy;

static int basic_open( struct inode *inode, struct file *file )
{
        printk( KERN_INFO "running_sum: device opened\n" );
        return 0;
}

static int basic_release( struct inode *inode, struct file *file )
{
        printk( KERN_INFO "running_sum : device closed" );
        return 0;
}

static ssize_t basic_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{

	int len;
        if(*offset > 0)
		return 0;

	len = scnprintf( result, sizeof(result), "%d\n", running_sum);

        if( copy_to_user( user_buffer , result , len ))
                return -EFAULT;
        printk(KERN_INFO "running sum: read %d bytes\n",bytes_to_copy);
        *offset = len;
	return len;
}

static void cal_run_sum(void)
{
	int n = 0,ret;
	
	ret = kstrtoint( buffer, 10, &n );
	if(ret < 0){
	  pr_err("Invalid input\n");
	  return;
	}

	printk (KERN_INFO "The atoi is %d",n);
	running_sum += n;
}

static ssize_t basic_write( struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
        bytes_to_copy = count;
        if(copy_from_user( &buffer , user_buffer, bytes_to_copy ))
                return -EFAULT;
        printk(KERN_INFO "Recieved data : string %s",buffer);
        buffer[bytes_to_copy] = '\0';
        cal_run_sum();
        printk( KERN_INFO "cal_char: wrote %d bytes\n", bytes_to_copy );
        return bytes_to_copy;
}

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = basic_open,
  .read = basic_read,
  .write = basic_write,
  .release = basic_release,
};

static int __init basic_init(void)
{
	major = register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("running sum driver: loaded, major = %d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("running sum driver:  unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");


