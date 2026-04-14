#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "case_conv_driver"

struct CASE_CONVERSION {
    char buf[128];
};

static struct CASE_CONVERSION case_conv;

static int major;

static void case_conversion(char *ptr)
{
	while(*ptr)
	{
		if(((*ptr >= 'A') && (*ptr <= 'Z')) || ((*ptr >= 'a') && (*ptr <= 'z')))
		{
			*ptr ^=32;
		}
		ptr++;
	}

}

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
        int bytes_to_copy = sizeof( struct CASE_CONVERSION );
        if( copy_to_user( user_buffer ,&case_conv , bytes_to_copy ))
                return -EFAULT;

        printk(KERN_INFO "cal_char: read %d bytes \n",bytes_to_copy);

        return bytes_to_copy;
}

static ssize_t basic_write( struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy = sizeof(struct CASE_CONVERSION);

        if(copy_from_user( &case_conv , user_buffer, bytes_to_copy ))
                return -EFAULT;

        printk(KERN_INFO "Recieved data : string %s",case_conv.buf);
        
        case_conversion(case_conv.buf);	
        
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
	pr_info("case-conv-driver: loaded, major = %d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("case-conv-drv:  unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");


