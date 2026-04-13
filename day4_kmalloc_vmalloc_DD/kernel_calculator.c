#include<linux/module.h>//core module macros
#include<linux/kernel.h>//printk()
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
int a,b,result;
char op[10];

bytes_to_copy = min(count, (size_t)BUF_SIZE);
/* copy data from user spac to kernel space */
if(copy_from_user(kernel_buffer,user_buffer, bytes_to_copy))
return -EFAULT;

kernel_buffer[bytes_to_copy]='\0';

sscanf(kernel_buffer,"%d %d %s",&a,&b,op);

if(strcmp(op,"ADD")==0)
result=a+b;

else if(strcmp(op,"SUB")==0)
result=a-b;

else if(strcmp(op,"MUL")==0)
result=a*b;

else if(strcmp(op,"DEV")==0)
result=a/b;

else
return bytes_to_copy;

buffer_size=snprintf(kernel_buffer,BUF_SIZE,"%d",result);
printk(KERN_INFO"basic_char: wrote %d bytes\n",bytes_to_copy);
return bytes_to_copy;
}
/* file operations structure
*this connects system calles to driver functions*/
static struct file_operations basic_flops = {
.owner=THIS_MODULE,
.open=basic_open,
.read=basic_read,
.write=basic_write,
.release=basic_release,
};

/* module initialization */
static int __init basic_char_init(void){
/* register character device 
*0->dynamic major number
*/

major_number=register_chrdev(0,DEVICE_NAME,&basic_flops);
if(major_number<0){
printk(KERN_ERR"basic_char: failed to register device\n");
return major_number;
}

printk(KERN_INFO"basic_char: loaded\n");
printk(KERN_INFO"basic_char: major number = %d\n",major_number);
printk(KERN_INFO"Create device node with:\n");
printk(KERN_INFO"mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);

return 0;
}
/* module cleanup*/

static void __exit basic_char_exit(void){
unregister_chrdev(major_number,DEVICE_NAME);
printk(KERN_INFO"Bbasic_char: unloaded\n");
}

/* these macros tell the kernel which functions should be called when the module is inserted and removed */

module_init(basic_char_init);
module_exit(basic_char_exit);

/* mandatory module metadata */
MODULE_LICENSE("GPL"); //PREVENTS KERNEL TAINT
MODULE_AUTHOR("VAISHNAVI");
MODULE_DESCRIPTION("Educational basic character driver with file operations");


