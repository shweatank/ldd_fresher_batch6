#include<linux/uaccess.h> //copy_from_user and copy_to_user
#include<linux/kernel.h> //min macro
#include<linux/fs.h>
#include<linux/init.h>
#include<linux/module.h>

#define BUFSIZE 256
#define DEVICE_NAME "basic_char"

static char kernel_buffer[BUFSIZE];
static int buffer_size;
static int major_number;

static int palindrome_open(struct inode *inode,struct file *file)
{
  printk("Device opened\n");
  return 0;
}

static int palindrome_release(struct inode *inode,struct file *file)
{
 printk("Device close\n");
 return 0;
}

static ssize_t basic_read(struct file *file,char __user *user_buffer ,size_t count,loff_t *offset)
{
     int bytes_to_copy;
     if(*offset >= BUFSIZE)
     {
	    return 0;
     }
      bytes_to_copy=min(count,(size_t)(BUFSIZE-*offset));
     if(copy_to_user(user_buffer,kernel_buffer+ *offset,bytes_to_copy))
     {
	    return -EFAULT;
     }
     *offset+=bytes_to_copy;

     pr_info("basic_char read %d bytes\n",bytes_to_copy);
     printk(KERN_INFO "Returning %s\n",kernel_buffer);
    return bytes_to_copy;

}

static ssize_t basic_write(struct file *file,const char __user *user_buffer ,size_t count,loff_t *offset)
{
     int bytes_to_copy;
     int len,i,j,a,b,temp;
     if(*offset >= BUFSIZE)
     {
            return 0;
     }
      bytes_to_copy=min(count,(size_t)BUFSIZE-1);
     if(copy_from_user(kernel_buffer,user_buffer,bytes_to_copy))
     {
            return -EFAULT;
     }
      kernel_buffer[bytes_to_copy]='\0';
     len=strlen(kernel_buffer);
     for(i=0;i<len-1;i++)
     {
	for(j=i+1;j<len;j++)
	{
	 a=kernel_buffer[i];
	 b=kernel_buffer[j];
	 if(a>b)
	 {
	  temp=kernel_buffer[i];
	  kernel_buffer[i]=kernel_buffer[j];
	  kernel_buffer[j]=temp;
	}
       }
     }
     buffer_size=bytes_to_copy;
     printk(KERN_INFO "Returning %s\n",kernel_buffer);

    return bytes_to_copy;
}

static struct file_operations fops={
  .owner=THIS_MODULE,
  .open=palindrome_open,
  .write=basic_write,
  .read=basic_read,
  .release=palindrome_release,
};

static int __init basic_char_init(void)
{
   major_number = register_chrdev(0,DEVICE_NAME,&fops);
   if(major_number < 0)
     {
           printk(KERN_ERR "basic_char: failed to register device\n");
           return major_number;
     }

     printk(KERN_INFO "basic_char: loaded\n");
     printk(KERN_INFO "basic_char: major number = %d\n", major_number);
     printk(KERN_INFO "Create device node with:\n");
     printk(KERN_INFO "mknod /dev/%s c %d 0\n", DEVICE_NAME, major_number);

     return 0;
}

static void __exit basic_char_exit(void)
{
  unregister_chrdev(major_number,DEVICE_NAME);
  printk(KERN_INFO "basic_char: unloaded\n");
}

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nandini");
MODULE_DESCRIPTION("Sorting integer array");
