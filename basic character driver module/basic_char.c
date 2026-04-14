#include <linux/module.h>   // Core kernel modules
#include <linux/kernel.h>   // Required for printk
#include <linux/init.h>     // Required for __init and __exit macros
#include <linux/fs.h>       // Register_chrdev, file_operations
#include <linux/uaccess.h>  // Copy_to_user, copy_from_user
	
#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256

static int major_number;
static char kernel_buffer[BUF_SIZE];
static int buffer_size;

/*
 * Called when user opens /dev/basic_char
*/

static int basic_open( struct inode *inode, struct file *file )
{
	printk( KERN_INFO "basic_char: device opened\n" );
	return 0;
}

/*
 * Called when user closes /dev/basic_char
*/

static int basic_release( struct inode *inode, struct file *file )
{
	printk( KERN_INFO "basic_char: device closed" );
	return 0;
}


/*
 * Called when user reads /dev/basic_char
*/

static ssize_t basic_read(struct file *file,char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy;
	/*
	 *if offset is beyond data, return 0 (EOF)
	 * */
	if(*offset >= buffer_size)
		return 0;
	bytes_to_copy = min(count, (size_t)(buffer_size - *offset));
	
	/*
	 *Copy data from kernel space to user space
	*/
        if(copy_to_user(user_buffer,kernel_buffer + *offset,bytes_to_copy))
		return -EFAULT;

	*offset += bytes_to_copy;

	printk(KERN_INFO "basic_char: read %d bytes \n",bytes_to_copy);

	return bytes_to_copy;
}

/*
 * Called when user write /dev/basic_char
*/

static void rev_str(char *l,char *r)
{
	while(l<r)
	{
		char temp = *l;
		*l = *r;
		*r = temp;
		l++,r--;
	}
} 

static ssize_t basic_write(struct file *file,const char __user *user_buffer,size_t count, loff_t *offset)
{
	int bytes_to_copy;
	
	bytes_to_copy = min(count, (size_t)BUF_SIZE);
	
	/*
	 *Copy data from user space to kernel space
	*/

        if(copy_from_user(kernel_buffer,user_buffer,bytes_to_copy))
		return -EFAULT;
	buffer_size = bytes_to_copy;
        rev_str( kernel_buffer, kernel_buffer + bytes_to_copy - 1);
	printk( KERN_INFO "basic_char: wrote %d bytes\n", bytes_to_copy );

	return bytes_to_copy;
}


/*
 *File operations structure
 *This connects system calls to driver functions
 * */

static struct file_operations basic_fops = {
  .owner = THIS_MODULE,
  .open = basic_open,
  .read = basic_read,
  .write = basic_write,
  .release = basic_release,
};

/*
 * Module initailization 
 */


static int __init basic_char_init(void)
{
	//register character device
	//0 -> dynamic major number
	
	major_number = register_chrdev(0,DEVICE_NAME,&basic_fops);

	if(major_number < 0)
	{
		printk(KERN_INFO "basic_char: failed to register device\n");
		return major_number;
	}
        printk(KERN_INFO "basic_char : module loaded\n");
	printk(KERN_INFO "basic_char: major number = %d\n",major_number);
	printk(KERN_INFO "Created device node with : \n");
	printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);

        return 0;  //Returning 0 means successful load
}

//module cleanup
//

static void __exit basic_char_exit(void)
{
	unregister_chrdev(major_number,DEVICE_NAME);
        printk(KERN_INFO "basic_char: unloaded\n");
}


//kernel module macros

module_init(basic_char_init);
module_exit(basic_char_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Almas");
MODULE_DESCRIPTION("Most basic linux kernel module for education");
