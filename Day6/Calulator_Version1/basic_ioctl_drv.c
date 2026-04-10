// Minimal IOCTL example : send an int from user ->kernal ->modify ->return 

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/ioctl.h>

#define DEVICE_NAME "basic_ioctl" 
#define IOCTL_MAGIC 'B'
struct calc {
    int a;
    int b;
    char op[10];
    int result;
};
#define IOCTL_CALC _IOWR(IOCTL_MAGIC, 1, struct calc)

static int major;
//static int kernel_value=0;

/* ioctl number */
static long basic_ioctl(struct file *file,unsigned int cmd , unsigned long arg){
 struct calc c;
 switch(cmd){
   case IOCTL_CALC:
	   /*copy data from user*/
	   if(copy_from_user(&c,(struct calc __user *)arg ,sizeof(struct calc)))
		   return -EFAULT;
	   if(strcmp(c.op , "add")==0){
	      c.result = c.a+c.b;
	   }
	   else if(strcmp(c.op , "sub")==0){
	      c.result = c.a-c.b;
	   }
	   else if(strcmp(c.op , "mul")==0){
	      c.result = c.a*c.b;
	   }
	   else if(strcmp(c.op , "div")==0){
	      if(c.b !=0){	   
	      c.result = c.a/c.b;
	      }
	      else{
	        pr_info("Can't Divide with Zero\n");
	      }
	   }
	   else{
	     c.result = 0;
	   }
	   
           pr_info("Kernel: %d + %d = %d\n", c.a, c.b, c.result);

	   /*kernel modifies data*/
	   if(copy_to_user((struct calc __user *)arg,&c ,sizeof(struct calc)))
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
