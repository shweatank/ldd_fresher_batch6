#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEVICE_NAME "sumdev"

static int sum = 0;
static int major_number;

static int sumdev_open(struct inode *inode, struct file *file) {
     printk(KERN_INFO "sumdev opened\n");
    return 0;
}

static int sumdev_release(struct inode *inode, struct file *file) {
      printk(KERN_INFO "sumdev released\n");
    return 0;
}

static ssize_t sumdev_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset) {
    int value;
    if (kstrtoint_from_user(user_buffer, count, 10, &value)) {
        return -EINVAL;
    }
    sum += value;
     printk(KERN_INFO "Added value: %d | Sum: %d\n", value, sum);
    return count;
}

static ssize_t sumdev_read(struct file *file,char __user *user_buffer,size_t count, loff_t *offset){
    char buffer[20];
    int len;

    if (*offset > 0)
        return 0;  // EOF

    len = sprintf(buffer, "%d\n", sum);
    if(count < len)
	  len = count;

    if (copy_to_user(user_buffer, buffer, len))
        return -EFAULT;

    *offset += len;

    return len;
}
static struct file_operations fops = {
    .open = sumdev_open,
    .release = sumdev_release,
    .write = sumdev_write,
    .read = sumdev_read,
};

static int __init sumdev_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "Failed to register character device\n");
        return major_number;
    }
    printk(KERN_INFO "Sumdev module loaded, MajorNumber =%d\n",major_number);
    return 0;
}

static void __exit sumdev_exit(void) {
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Sumdev module unloaded\n");
}

module_init(sumdev_init);
module_exit(sumdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Running Sum Device Driver");
