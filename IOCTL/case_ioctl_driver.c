// basic_ioctl_drv.c
// Minimal IOCTL example: send an int from user → kernel → modify → return
 
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define SIZE 100
#define DEVICE_NAME "ioctl_example"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC, 1, char *)
 
static int major;
static char user_str[SIZE];
 
/* ioctl handler */
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
    // int user_value;

     switch (cmd) {
    	     case IOCTL_SET_VALUE:
             /* copy data from user */
             if (copy_from_user(&user_str, (char __user *)arg, sizeof(char)*SIZE))
                 return -EFAULT;
 
             pr_info("kernel: received %s from user\n", user_str);
 
             /* kernel modifies data */
             //kernel_value = user_value * 10;

	     for(int i=0;i<strlen(user_str);i++)
	     {
	     	if(user_str[i]>='a'&&user_str[i]<='z')
		{
			user_str[i]=user_str[i]-32;
		}
		else if(user_str[i]>='A'&&user_str[i]<='Z')
		{
			user_str[i]=user_str[i]+32;
		}
	     }
 
             /* copy data back to user */
             if (copy_to_user((char __user *)arg, user_str, strlen(user_str)))
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
  
 static int __init basic_init(void)
 {
     major = register_chrdev(0, DEVICE_NAME, &fops);
     pr_info("basic_ioctl loaded, major=%d\n", major);
     return 0;
 }
 
 static void __exit basic_exit(void)
 {
     unregister_chrdev(major, DEVICE_NAME);
     pr_info("basic_ioctl unloaded\n");
 }
  
 module_init(basic_init);
 module_exit(basic_exit);
  
 MODULE_LICENSE("GPL");
