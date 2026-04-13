#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>
#include<linux/slab.h>

#define DEVICE_NAME "dyn_ioctl"
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_SIZE _IOWR(IOCTL_MAGIC,1,int)
#define BUF_SIZE 100


static int major;

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

static ssize_t basic_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset){
int bytes_to_copy;

bytes_to_copy = min(count, (size_t)BUF_SIZE);
/* copy data from user spac to kernel space */
if(copy_from_user(kernel_buffer,user_buffer, bytes_to_copy))
return -EFAULT;

buffer_size=bytes_to_copy;
printk(KERN_INFO"basic_char: wrote %d bytes\n",bytes_to_copy);
return bytes_to_copy;
}

/*ioctl handler*/
static long dyn_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
int size;


switch(cmd){
case IOCTL_SET_SIZE:
/* copy data from user */
if(copy_from_user(&size,(int __user*)arg, sizeof(int)))
return -EFAULT;

/* validate size */
if(data.size<=0 || data.size>MAX_SIZE)
{
printk("Invalid size\n");
return -EINVAL;
}

data.sum=0;
for(int i=0;i<data.size;i++){
data.sum+=data.a[i];
}

data.avg = data.sum/data.size;

/* copy data back to user */
if(copy_to_user((struct array_data __user *)arg,&data,sizeof(data)))
return -EFAULT;

break;
default:
return -EINVAL;

}

return 0;
}

static struct file_operations fops={
.owner=THIS_MODULE,
.unlocked_ioctl = array_ioctl,
};

static int __init basic_init(void){
major=register_chrdev(0,DEVICE_NAME,&fops);
pr_info("calc_ioctl loaded, major=%d\n",major);
return 0;
}
static void __exit basic_exit(void){
unregister_chrdev(major,DEVICE_NAME);
pr_info("calc_ioctl unloaded\n");
}
module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
