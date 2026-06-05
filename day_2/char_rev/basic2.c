#include <linux/module.h>  //core module macros
#include <linux/kernel.h>  //printk()
#include <linux/init.h>   //__init, __exit
#include <linux/fd.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256

static int major_number;
static char kernel_buffer[BUF_SIZE];
static int buffer_size;
static int len;


static int basic_open(struct inode *inode,struct file *file)
{

	printk(KERN_INFO "basic_char:device opened\n");
	return 0;

}
//called when user closes /dev/basic_char

static int basic_release(struct inode *inode,struct file *file)
{
	printk(KERN_INFO "basic_char: device closed\n");
	return 0;

}

//called when user reads from 
//

static ssize_t basic_read(struct file *file,char __user *user_buffer,
					size_t count,loff_t *offset)


{          
	int bytes_to_copy;
        //char str[50];
	//int len_1=0;
	//if offset is beyond data, return 0 (EOF)
	if(*offset >= buffer_size)
	return 0;
	//sscanf(kernel_buffer,"%d,%d,%c",&val_1,&val_2,&op);
	//sscanf(kernel_buffer,"%s",str);
	bytes_to_copy = min(count,(size_t)(buffer_size - *offset));
        

	//sprintf(kernel_buffer,"%d",res);
	int i;
	for(i=0;kernel_buffer[i];i++){
		if(kernel_buffer[i]!=' '){
	kernel_buffer[i]=kernel_buffer[i]^32;
		}
	}

	
       // kernel_buffer[0]=((kernel_buffer[0]-48)+(kernel_buffer[2]-48));
	
	if(copy_to_user(user_buffer,kernel_buffer +*offset,bytes_to_copy))
		return -EFAULT;
	*offset += bytes_to_copy;
	printk(KERN_INFO "basic_char: read %d bytes\n",bytes_to_copy);
	return bytes_to_copy;
}

static ssize_t basic_write(struct file *file,const char __user *user_buffer,
			size_t count,loff_t *offset)
{
	int bytes_to_copy;
	bytes_to_copy=min(count,(size_t)BUF_SIZE);
	//copy the data from user space to kernal space
	len=bytes_to_copy;
	len=len-2;
	if(copy_from_user(kernel_buffer,user_buffer,bytes_to_copy))
		return -EFAULT;

	buffer_size=bytes_to_copy;

	printk(KERN_INFO "basic_char: wrote %d bytes\n",bytes_to_copy);
	return bytes_to_copy;
}

//file operations structure
//this connects system calls to driver functions

static struct file_operations basic_fops={
.owner = THIS_MODULE,
.open = basic_open,
.read = basic_read,
.write = basic_write,
.release = basic_release,
};

//module intilization

static int __init basic_char_init(void)
{
	//register character device
	//0->dynamic major number
	
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
	
	return 0;

}


//module cleanup

static void __exit basic_char_exit(void)
{
	unregister_chrdev(major_number,DEVICE_NAME);
	printk(KERN_INFO "basic_char: unloaded\n");
}

//kernel module macros

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pavan");
MODULE_DESCRIPTION("Educatiional basic character driver with file operations");

