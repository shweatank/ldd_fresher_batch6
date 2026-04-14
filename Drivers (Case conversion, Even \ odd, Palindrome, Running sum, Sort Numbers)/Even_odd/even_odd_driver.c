#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "even_odd_driver"

static int result;

static int major;

static int basic_open( struct inode *inode, struct file *file )
{
        printk( KERN_INFO "cal_char: device opened\n" );
        return 0;
}

// Called when user closes /dev/basic_char
static int basic_release( struct inode *inode, struct file *file )
{
        printk( KERN_INFO "cal_char: device closed" );
        return 0;
}

static ssize_t basic_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy = sizeof( int );
        if( copy_to_user( user_buffer ,&result , bytes_to_copy ))
                return -EFAULT;

        printk(KERN_INFO "cal_char: read %d bytes \n",bytes_to_copy);

        return bytes_to_copy;
}

static ssize_t basic_write( struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy = sizeof( int );

        if(copy_from_user( &result , user_buffer, bytes_to_copy ))
                return -EFAULT;

        printk(KERN_INFO "Recieved data : %d",result);

        result &= 1; 

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
	pr_info("even odd driver: loaded, major = %d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("even odd -drv:  unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");


