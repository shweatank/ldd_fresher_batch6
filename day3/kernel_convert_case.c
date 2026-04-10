#include<linux/module.h> //Core module macros			 
#include<linux/kernel.h> //printk()
#include<linux/init.h>   // __init,__exit
#include<linux/fs.h>  //register_chrdev,file_operations
#include<linux/uaccess.h> //copy_to_user,copy_from_user
#include<linux/slab.h> //kamlloc,free;

#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256

static int major_number;
static char *kernel_buffer;
static int buffer_size;

/*
 * Called when user opens /dev/basic_char
 */

static int basic_open(struct inode *inode,struct file *file)
{
	//printk(KERN_INFO "basic_char:device opened\n");
	printk(KERN_INFO "I am opened\n");
	return 0;
}

/*
 * called when user closes /dev/basic_char
 */

static int basic_release(struct inode *inode,struct file *file)
{
	//printk(KERN_INFO "basic_char:device closed\n");
	printk(KERN_INFO "I am closed\n");
	return 0;
}

/*
 * Called when user reads from /dev/basic_char
 */

static ssize_t basic_read(struct file *file,char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy;

        /*
         * If offset is beyond data,return 0 (EOF)
         */
        if(*offset >= buffer_size)
                return 0;
        bytes_to_copy=min(count,(size_t)(buffer_size - *offset));

        /*
         * copy data from kernel space to user space
         */
        if(copy_to_user(user_buffer,kernel_buffer + *offset,bytes_to_copy))
                return -EFAULT;
        *offset += bytes_to_copy;
        printk(KERN_INFO "I am reading the characters\n");
        printk(KERN_INFO "basic char:read %d bytes\n",bytes_to_copy);
        return bytes_to_copy;
}

/*
 * called when user writes to /dev/basic_char
 */

static ssize_t basic_write(struct file *file,const char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy;
	bytes_to_copy=min(count,(size_t)BUF_SIZE);

	/*
	 * copy data from user space to kernel space
	 */
	if(copy_from_user(kernel_buffer,user_buffer,bytes_to_copy))
		return -EFAULT;
	kernel_buffer[bytes_to_copy]='\0';
	printk(KERN_INFO "Received :%s\n",kernel_buffer);
	for(int i=0;i<bytes_to_copy;i++)
	{
		if(kernel_buffer[i]>='a'&&kernel_buffer[i]<='z')
			kernel_buffer-=32;
		else if(kernel_buffer[i]>='A'&&kernel_buffer[i]<='Z')
			kernel_buffer+=32;
	}
	buffer_size=bytes_to_copy;
	printk(KERN_INFO "converted=%s\n",kernel_buffer);
	printk(KERN_INFO "buffer size=%d\n",buffer_size);
	printk(KERN_INFO "I am writing the charcters\n");
	printk(KERN_INFO "basic_char:write %d bytes\n",bytes_to_copy);
	return bytes_to_copy;
}

/*
 * file operations structure
 * this connects system calls to driver functions
 */

static struct file_operations basic_fops={
	.owner=THIS_MODULE,
	.open=basic_open,
	.read=basic_read,
	.write=basic_write,
	.release=basic_release,
};
/*
 * module instructions
 */
static int __init basic_char_init(void)
{
	/*
	 * register character device
	 * 0 ->dynamic major number
	 */
	major_number=register_chrdev(0,DEVICE_NAME,&basic_fops);
	if(major_number<0)
	{
		printk(KERN_INFO "basic_char:faile dto register device\n");
		return major_number;
	}
	kernel_buffer=kmalloc(BUF_SIZE,GFP_KERNEL);
	if(!kernel_buffer)
	{
		printk(KERN_ALERT "Memory allocation failed\n");
		unregister_chrdev(major_number,DEVICE_NAME);
		return -ENOMEM;
	}
	printk(KERN_INFO "basic char:Memory allocated\n"); 
	printk(KERN_INFO "basic_char:loaded\n");
	printk(KERN_INFO "basic_char:major number=%d\n",major_number);
	printk(KERN_INFO "Create device node with:\n");
	printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);
	return 0;
}
/*
 * module cleanup
 */
static void __exit basic_char_exit(void)
{
	if(kernel_buffer)
	{
		kfree(kernel_buffer);
		printk(KERN_INFO "Memory freed\n");
	}
	unregister_chrdev(major_number,DEVICE_NAME);
	printk(KERN_INFO "basic_char:unloaded\n");
}
/*kernel module macros*/
module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sarika");
MODULE_DESCRIPTION("Educational basic character driver with file operations");
