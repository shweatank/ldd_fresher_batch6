/*to check whether a number is even or odd*/
#include<linux/init.h>//__init, __exit
#include<linux/fs.h>//register chrdev, file_operations
#include<linux/uaccess.h>//copy_to_user,copy_from_user

#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256

static int major_number;
static char kernel_buffer[BUF_SIZE];
static int buffer_size;

/*
*Called when user opens/dev/basic_char
*/

static int basic_open(struct inode *inode, struct file *file)
{
printk(KERN_INFO"basic_char: device opened\n");
return 0;
}

/* Called when user closes /dev/basic_char */
static int basic_release(struct inode *inode,struct file *file)
{
printk(KERN_INFO"basic_char: device closed\n");
return 0;
}
/*
called when user reads from /dev/basic_char */
static ssize_t basic_read(struct file *file, char __user *user_buffer, size_t count,loff_t *offset){
int bytes_to_copy;
/* if offset is beyond data, return 0 (EOF) */
if(*offset >= buffer_size)
return 0;

bytes_to_copy=min(count, (size_t)(buffer_size - *offset));

/* copy data from kernel space to user space */
if(copy_to_user(user_buffer,kernel_buffer + *offset, bytes_to_copy))
return -EFAULT;

*offset += bytes_to_copy;
printk(KERN_INFO"basic-char: read %d bytes\n",bytes_to_copy);
return bytes_to_copy;
}

/* called when user writes to /dev/basic_char */
static ssize_t basic_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset){
int bytes_to_copy;

bytes_to_copy = min(count, (size_t)BUF_SIZE);
/* copy data from user spac to kernel space */
if(copy_from_user(kernel_buffer,user_buffer, bytes_to_copy))
return -EFAULT;

int n;
int ret;
kernel_buffer[bytes_to_copy]='\0';
ret=kstrtoint(kernel_buffer,10,&n);
if(ret<0){
printk("conversion failed!\n");
return ret;
}

if(n%2==0)
buffer_size=snprintf(kernel_buffer,BUF_SIZE,"Even\n");
else
buffer_size=snprintf(kernel_buffer,BUF_SIZE,"Odd\n");


printk(KERN_INFO"basic_char: wrote %d bytes\n",bytes_to_copy);
return bytes_to_copy;
}
