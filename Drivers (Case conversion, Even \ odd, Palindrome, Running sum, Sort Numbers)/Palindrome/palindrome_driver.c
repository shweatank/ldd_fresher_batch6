#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "palindrome_ioctl"

struct palindrome_t {
    char buf[128];
    int status;
};

static struct palindrome_t req;

static int major;

static int my_strlen(char *str)
{
      int count = 0;
      while(*str)
      {
	      count++;
	      str++;
      }
      return count;
}

static int isPalindrome(char *l, char *r)
{
	while(l<r)
	{
		if(*l != *r)
			return 0;
		l++,r--;
	}
	return 1;

}

static int basic_open( struct inode *inode, struct file *file )
{
        printk( KERN_INFO "palindrome_driver : device opened\n" );
        return 0;
}


// Called when user closes /dev/basic_char
static int basic_release( struct inode *inode, struct file *file )
{
        printk( KERN_INFO "palindrome driver : device closed" );
        return 0;
}

static ssize_t basic_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy = sizeof( struct palindrome_t );
        if( copy_to_user( user_buffer ,&req , bytes_to_copy ))
                return -EFAULT;

        printk(KERN_INFO "palindrome driver: read %d bytes \n",bytes_to_copy);

        return bytes_to_copy;
}

static ssize_t basic_write( struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy = sizeof(struct palindrome_t);

        if(copy_from_user( &req , user_buffer, bytes_to_copy ))
                return -EFAULT;

        printk(KERN_INFO "Recieved data : string %s",req.buf);

        req.status = isPalindrome( req.buf, req.buf + my_strlen(req.buf) - 1);

        printk( KERN_INFO "palindrome driver : wrote %d bytes\n", bytes_to_copy );

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
	pr_info("palindrome-driver: loaded, major = %d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("palindrome-driver :  unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");


