// Minimal IOCTL example : send an int from user ->kernal ->modify ->return 

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/ioctl.h>

#define DEVICE_NAME "basic_ioctl" 
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC , 1, int)

static int major;
static int kernel_value=0;

/* ioctl number */
static long basic_ioctl(struct file *file,unsigned int cmd , unsigned long arg){
 int user_value;
 switch(cmd){
   case IOCTL_SET_VALUE:
	   /*copy data from user*/
	   if(copy_from_user(&user_value,(int __user *)arg ,sizeof(int)))
		   return -EFAULT;
	   pr_info("Kernel : received %d from user\n",user_value);

	   /*kernel modifies data*/
	   kernel_value = user_value + 10 ;
	   if(copy_to_user((int __user *)arg,&kernel_value ,sizeof(int)))
		   return -EFAULT;
	   break;
   default:
	   return -EINVAL;
 }
 return 0;
}
static struct file_operations fops = {
   .owner = THIS_MODULE,
   .unlocked_ioctl = basic_ioctl,
};
static int __init basic_init(void){
  major = register_chrdev(0,DEVICE_NAME,&fops);
  pr_info("Basic_ioctl loaded, major=%d\n",major);
  printk(KERN_INFO "basic_char : Loaded\n");
    printk(KERN_INFO "basic_char : Major Number: %d\n",major);
    printk(KERN_INFO "Create Device Node with\n");
    printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major);


  return 0;
}
static void __exit basic_exit(void){
  unregister_chrdev(major,DEVICE_NAME);
  pr_info("Basic_ioctl unloaded\n");
}
module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
