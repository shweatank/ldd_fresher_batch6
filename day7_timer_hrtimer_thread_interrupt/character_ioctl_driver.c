/* my own device to set mode, count write call, and clear buffer */
#include<linux/module.h>//core module macros
#include<linux/kernel.h>//printk()
#include<linux/init.h>//__init, __exit
#include<linux/fs.h>//register chrdev, file_operations
#include<linux/cdev.h>
#include<linux/uaccess.h>//copy_to_user,copy_from_user
#include<linux/mutex.h>
#include<linux/ioctl.h>

#define DEVICE_NAME "my_device"
#define BUF_SIZE 256
#define IOCTL_MAGIC 'E'
#define SET_MODE _IOW(IOCTL_MAGIC,1,int)
#define GET_MODE _IOR(IOCTL_MAGIC,2,int)
#define CLEAR_BUFFER _IO(IOCTL_MAGIC,3)
#define GET_WRITE_COUNT _IOR(IOCTL_MAGIC,4,int)

static int major_number;
static char kernel_buffer[BUF_SIZE];
static int device_mode=0;
static int write_count=0;

static struct mutex lock;


//open file
static int dev_open(struct inode *inode, struct file *file)
{
printk(KERN_INFO"my_device: device opened\n");
return 0;
}

/* Called when user closes /dev/my_device */

static int dev_release(struct inode *inode,struct file *file)
{
printk(KERN_INFO"my_device: device closed\n");
return 0;
}
/*
called when user reads from /dev/my_device */
static ssize_t dev_read(struct file *file, char __user *user_buffer, size_t count,loff_t *offset){

/* if offset is beyond data, return 0 (EOF) */
if(*offset >= BUF_SIZE)
return 0;

/* copy data from kernel space to user space */
if(copy_to_user(user_buffer,kernel_buffer,BUF_SIZE))
return -EFAULT;

*offset += BUF_SIZE;
return BUF_SIZE;
}

/* called when user writes to /dev/my_device */
static ssize_t dev_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset){
mutex_lock(&lock);

if(count>BUF_SIZE)
count=BUF_SIZE;

/* copy data from user spac to kernel space */
if(copy_from_user(kernel_buffer,user_buffer, count))
{
mutex_unlock(&lock);
return -EFAULT;
}
write_count++;

mutex_unlock(&lock);

printk(KERN_INFO"my_device:write count:  %d\n",write_count);
return count;
}

//ioctl
static long dev_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
int value;
switch(cmd){
case SET_MODE:
	if(copy_from_user(&value,(int __user*)arg,sizeof(value)))
	return -EFAULT;
	device_mode=value;
	printk("mode set to %d\n",device_mode);
	break;
case GET_MODE:
        if(copy_to_user((int __user*)arg,&device_mode,sizeof(device_mode)))
        return -EFAULT;
        break;
case CLEAR_BUFFER:
	memset(kernel_buffer,0,BUF_SIZE);
        printk("Buffer cleared\n");
        break;
case GET_WRITE_COUNT:
        if(copy_to_user((int __user*)arg,&write_count,sizeof(write_count)))
        return -EFAULT;
        break;
default:
	return -EINVAL;
}
return 0;
}

/* file operations structure
*this connects system calles to driver functions*/
static struct file_operations fops = {
.owner=THIS_MODULE,
.open=dev_open,
.read=dev_read,
.write=dev_write,
.release=dev_release,
.unlocked_ioctl=dev_ioctl
};
/* module initialization */
static int __init my_driver_init(void){
major_number=register_chrdev(0,DEVICE_NAME,&fops);
mutex_init(&lock);

printk(KERN_INFO"my_device: loaded\n");
printk(KERN_INFO"my_device: major number = %d\n",major_number);
printk(KERN_INFO"Create device node with:\n");
printk(KERN_INFO"mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);

return 0;
}
/* module cleanup*/

static void __exit my_driver_exit(void){
unregister_chrdev(major_number,DEVICE_NAME);
printk(KERN_INFO"my_device: unloaded\n");
}

/* these macros tell the kernel which functions should be called when the module is inserted and removed */

module_init(my_driver_init);
module_exit(my_driver_exit);

/* mandatory module metadata */
MODULE_LICENSE("GPL"); //PREVENTS KERNEL TAINT

