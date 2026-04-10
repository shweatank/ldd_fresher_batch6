// Minimal IOCTL example : send an int from user ->kernal ->modify ->return 

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/ioctl.h>

#define DEVICE_NAME "basic_ioctl" 
struct calc {
    int a;
    int b;
    int result;
};
#define IOCTL_MAGIC 'C'
#define IOCTL_ADD _IOWR(IOCTL_MAGIC, 1, struct calc)
#define IOCTL_SUB _IOWR(IOCTL_MAGIC, 2, struct calc)
#define IOCTL_MUL _IOWR(IOCTL_MAGIC, 3, struct calc)
#define IOCTL_DIV _IOWR(IOCTL_MAGIC, 4, struct calc)

static int major;
//static int kernel_value=0;
static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct calc c;

    // Always copy from user first
    if (copy_from_user(&c, (struct calc __user *)arg, sizeof(struct calc))) {
        return -EFAULT;
    }

    switch(cmd) {
        case IOCTL_ADD:
            c.result = c.a + c.b;
            break;
        case IOCTL_SUB:
            c.result = c.a - c.b;
            break;
        case IOCTL_MUL:
            c.result = c.a * c.b;
            break;
        case IOCTL_DIV:
            if (c.b != 0) {
                c.result = c.a / c.b;
            } else {
                pr_err("Kernel: Division by zero attempted\n");
                return -EINVAL; 
            }
            break;
        default:
            return -EINVAL;
    }

    pr_info("Kernel: Operation on %d and %d result is %d\n", c.a, c.b, c.result);

    // Copy the updated struct (with result) back to user space
    if (copy_to_user((struct calc __user *)arg, &c, sizeof(struct calc))) {
        return -EFAULT;
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
