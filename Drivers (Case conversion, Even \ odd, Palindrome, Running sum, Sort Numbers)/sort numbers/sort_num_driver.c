#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "cal_ioctl"

struct array_t {
        int arr[50];
	int size;
};

static struct array_t req;

static int major;

static void sort_array(int *arr, int size)
{
	for(int i = 0; i < size - 1; i++){
		for(int j = 0; j < size - i - 1; j++ )
                {
			if(arr[j] > arr[ j + 1 ])
			{
				int temp = arr[j];
				arr[j] = arr[j+1];
				arr[j+1] = temp;
			}
	        }
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
        int bytes_to_copy = sizeof( struct array_t );
        if( copy_to_user( user_buffer ,&req , bytes_to_copy ))
                return -EFAULT;

        printk(KERN_INFO "cal_char: read %d bytes \n", bytes_to_copy);

        return bytes_to_copy;
}

static ssize_t basic_write( struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy = sizeof( struct array_t );

        if(copy_from_user( &req , user_buffer, bytes_to_copy ))
                return -EFAULT;

        printk(KERN_INFO "Recieved data \n");

        sort_array( req.arr, req.size );

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
	pr_info("basic_ioctl loaded, major = %d\n",major);
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


