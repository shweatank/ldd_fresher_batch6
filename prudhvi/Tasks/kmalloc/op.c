#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/string.h>
#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256

static int major_number;
static char *ptr;
static int buffer_size;
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prudhvi");
MODULE_DESCRIPTION("Educational basic character driver with file operation");

/*
 *called when user opens /dev/basic_char
 */

static int basic_open(struct inode *inode,struct file *file)
{
	printk(KERN_INFO "basic_char: device opened\n");
	return 0;
}

/*
 *called when user closes dev/nasic_char
 */

static int basic_release(struct inode *inode,struct file *file)
{
	printk(KERN_INFO "basic_char: device closed\n");
	return 0;
}
static ssize_t basic_read(struct file *file,char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy;
	/*
	   if offset is beyond data, return 0 (EOF)
	 */

	if(*offset >= buffer_size)
		return 0;

	bytes_to_copy = min(count,(size_t)(buffer_size - *offset));

	/*
	   copy data from kernel space to user space
	 */
	if(copy_to_user(user_buffer,ptr + *offset,bytes_to_copy))
		return -EFAULT;

	*offset += bytes_to_copy;

	printk(KERN_INFO "basic_char: read %d bytes\n",bytes_to_copy);
	return bytes_to_copy;
}

/*
   called when user writes to dev/basic_char
 */

static ssize_t basic_write(struct file *file,const char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy;
	int n1=0,n2=0,sum=0;
	char temp[BUF_SIZE];
bytes_to_copy = min(count, (size_t)BUF_SIZE);
	/*
	   copy data from the user space to kernel space 
	 */

	if(copy_from_user(temp,user_buffer,bytes_to_copy))
		return -EFAULT;

	temp[bytes_to_copy]='\0';
	char s[10];
	if(sscanf(temp,"%d %d %s",&n1,&n2,s)<3)
	{
		printk("two numbers is madathary...\n");
		return -EINVAL;
	}
	if(strcmp(s,"add")==0)
		sum=n1+n2;
	else if(strcmp(s,"sub")==0)
		sum=n1-n2;
	else if(strcmp(s,"mul")==0)
		sum=n1*n2;
	else{
		printk("Invalid operation\n");
		return -EINVAL;
	}
	buffer_size = snprintf(ptr,BUF_SIZE,"%d\n",sum);
	printk(KERN_INFO "basic_char: wrote %d bytes\n",bytes_to_copy);
	return bytes_to_copy;
}

static struct file_operations basic_fops = {
	.owner = THIS_MODULE,
	.open = basic_open,
	.read = basic_read,
	.write = basic_write,
	.release = basic_release
};

/*
   Module intialiazation
 */

static int __init basic_char_init(void)
{
	/*
	   register char device
	   0 ---> dynamic major number
	 */

	major_number = register_chrdev(0,DEVICE_NAME,&basic_fops);
	if(major_number < 0)
	{
		printk(KERN_ERR "basic_char: failed to register device\n");
		return major_number;
	}
ptr = (char *)kmalloc(BUF_SIZE, GFP_KERNEL);	
 if (!ptr) {
        printk(KERN_ALERT "Memory allocation failed\n");
        return -ENOMEM;
    }

	printk(KERN_INFO "basic_char: loaded\n");
	printk(KERN_INFO "basic_char: major  number = %d\n",major_number);
	printk(KERN_INFO "create device node with:\n");
	printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);
	return 0;
}

/*
   module cleanup
 */

static void __exit basic_char_exit(void)
{
	unregister_chrdev(major_number,DEVICE_NAME);
   if (ptr) {
        kfree(ptr);
        printk(KERN_INFO "Memory freed\n");
    }
	printk(KERN_INFO "basic char: unloaded\n");
}

/*
   kernel module macros
 */

module_init(basic_char_init);
module_exit(basic_char_exit);

