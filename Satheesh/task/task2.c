#include<linux/module.h> //core module access
#include<linux/kernel.h> //printk()
#include<linux/init.h> //__init,__exit
#include<linux/fs.h>  //register_chrdev, file_operations
#include<linux/uaccess.h> //copy_to_user,copy_from_use=r
#include<linux/vmalloc.h>

#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256
static int index=0;
static int major_number;
static char kernel_buffer[BUF_SIZE];
static int buffer_size;
char *ptr;
/* 
   *called when user opens /dev/basic_char
   */

static int basic_open(struct inode *inode,struct file *file)
{
        printk(KERN_INFO "basic_char: device opened\n");
        return 0;
}

/*
   *called when user closes /dev/basic_char
   */

static int basic_release(struct inode *inode,struct file *file)
{
        printk(KERN_INFO "basic_char: device close\n");
        return 0;
}

/*
   *called when user reads from /dev/basic_char
   */

static ssize_t basic_read(struct file *file,char __user *user_buffer,
                size_t count,loff_t *offset)
{
        int bytes_to_copy;

        /*
           *if offset is beyond data,return 0(EOF)
           */

        if(*offset >= buffer_size)
                return 0;


        bytes_to_copy=min(count,(size_t)(buffer_size - *offset));

        /*
           *copy data from kernel space to user space
           */

        if(copy_to_user(user_buffer,ptr + *offset,bytes_to_copy))
	return -EFAULT;

        *offset += bytes_to_copy;

        printk(KERN_INFO "basic_char: read %d bytes\n",bytes_to_copy);
       // printk(KERN_INFO "basic_char: copied to user %s\n",user_buffer);
        return bytes_to_copy;
}

/*
   *called when user writes to /dev/basic_char
   */

static ssize_t basic_write(struct file *file,const char __user *user_buffer,
                size_t count,loff_t *offset)
{
        int bytes_to_copy;
       bytes_to_copy=min(count,(size_t)BUF_SIZE);
	/*
        *COPY DATA FROM USER SPACE TO KERNEL SPACE
          */
       if(copy_from_user((ptr+index),user_buffer,bytes_to_copy))
               return -EFAULT;
	index+=bytes_to_copy;
       //printk(KERN_INFO"In kernel converted string :%s\n",ptr);

	/*
       if(copy_from_user,&result,bytes_to_copy))
               return -EFAULT;*/

       buffer_size=bytes_to_copy;

       printk(KERN_INFO "basic_char: wrote %d bytes\n",bytes_to_copy);
       //printk(KERN_INFO "basic_char: received string :%s\n",kernel_buffer);
       return bytes_to_copy;
}


/*
   *file operartions structure
   *this connects system calls to driver functions
   */


static struct file_operations basic_fops = {
        .owner=THIS_MODULE,
        .open=basic_open,
        .read=basic_read,
        .write=basic_write,
        .release=basic_release,
};

/*
   *module initialization
   */

static int __init basic_char_init(void)
{
        /*
           *register character device
           * 0 ->dynamic major number
           */

        major_number=register_chrdev(0,DEVICE_NAME,&basic_fops);
        if(major_number <0)
        {
                printk(KERN_ERR "basic_char : failed to register device\n");
                return major_number;
        }
	ptr=(char *)vmalloc(BUF_SIZE);
	if(!ptr)
	{
		printk(KERN_ALERT"Memory Allocation Failed\n");
		return -ENOMEM;
	}
        printk(KERN_INFO "basic_char : loaded\n");
        printk(KERN_INFO "basic_char : major number =%d\n",major_number);
        printk(KERN_INFO "create device node with:\n");
        printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);
        return 0;
}

/*
   *module cleanup
   */

static void __exit basic_char_exit(void)
{
        unregister_chrdev(major_number,DEVICE_NAME);
        printk(KERN_INFO "basic_char: unloaded\n");
	vfree(ptr);
}

/*kernel module macros*/

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prashant");
MODULE_DESCRIPTION("Educational basic character driver with file operartions");

