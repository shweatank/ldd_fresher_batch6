#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>

#include "ioctl_header.h"

#define DEVICE_NAME "IOCTL_CONTROL_INTERFACE"

static int major;

static struct control_io io_ctl = {.device_mode = 1, .wr_counter = 0, .buf_len = 0};

static long io_control(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case SET_MODE :
		        int mode;	
			if(copy_from_user( &mode, (int __user*)arg, sizeof(int) ))
				return -EFAULT;
			io_ctl.device_mode = mode;
			printk(KERN_INFO "%s : Mode set successfully to %d\n", DEVICE_NAME, mode);
			break;

		case GET_MODE :
			if(copy_to_user((int __user *)arg, &io_ctl.device_mode, sizeof(int)))
				return -EFAULT;
			printk(KERN_INFO "%s : Mode Get completed\n", DEVICE_NAME);
			break;

		case CLEAR_BUFFER :
			io_ctl.buffer[0] = '\0';
			io_ctl.buf_len = 0;
			printk(KERN_INFO "%s : Buffer cleared\n", DEVICE_NAME);
                        break;

		case GET_WRITE_COUNT :
			if(copy_to_user((int __user*)arg, &io_ctl.wr_counter, sizeof(int)))
				return -EFAULT;
			printk(KERN_INFO "%s : Fetched write count successfully : %d \n", DEVICE_NAME, io_ctl.wr_counter);
                        break;

	}
	return 0;
}

static int my_open( struct inode *inode, struct file *file )
{
        printk( KERN_INFO "cal_char: device opened\n" );
        return 0;
}


// Called when user closes /dev/basic_char
static int my_release( struct inode *inode, struct file *file )
{
        printk( KERN_INFO "cal_char: device closed" );
        return 0;
}

static ssize_t my_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy = count;

	
        if( copy_to_user( user_buffer , io_ctl.buffer , bytes_to_copy ))
                return -EFAULT;

        printk(KERN_INFO "cal_char: read %d bytes \n",bytes_to_copy);

        return bytes_to_copy;
}

static ssize_t my_write( struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy = count;
       
        if(copy_from_user( io_ctl.buffer , user_buffer, bytes_to_copy ))
                return -EFAULT;
         io_ctl.wr_counter++;
        printk(KERN_INFO "Recieved data : string %s",io_ctl.buffer);

        return bytes_to_copy;
}

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = my_open,
  .read = my_read,
  .write = my_write,
  .release = my_release,
  .unlocked_ioctl = io_control,
};

static int __init my_init(void)
{
        major = register_chrdev( 0, DEVICE_NAME, &fops);
        pr_info("%s: loaded, major = %d\n", DEVICE_NAME, major);
        return 0;
}

static void __exit my_exit(void)
{
        unregister_chrdev(major,DEVICE_NAME);
        pr_info("%s :  unloaded\n",DEVICE_NAME);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Almas");

