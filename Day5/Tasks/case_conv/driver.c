#include<linux/module.h> //requ for all kernal modules
#include<linux/kernel.h> // for printk
#include<linux/init.h>  // for __int and __exit
#include<linux/fs.h>  // register_chrdev , file_opearations
#include<linux/uaccess.h>  // copy_to_user , copy_from_user

MODULE_LICENSE("GPL");

#define DEVICE_NAME "add_char"
#define BUF_SIZE 256

static int major_number;
static char kernal_buffer[BUF_SIZE];
static int buffer_size;

static int basic_open(struct inode *inode,struct file *file){
  printk(KERN_INFO "Basic kernal module opened\n");
  return 0;
}
static int basic_release(struct inode *inode,struct file *file){
  printk(KERN_INFO "Basic kernal module closed\n");
  return 0;
}
static ssize_t basic_read(struct file *file, char __user *user_buffer,
                          size_t count, loff_t *offset)
{
    size_t bytes_to_copy;

    if (*offset >= buffer_size)
        return 0;

    bytes_to_copy = min(count, (size_t)(buffer_size - *offset));

    if (copy_to_user(user_buffer, kernal_buffer + *offset, bytes_to_copy))
        return -EFAULT;

    *offset += bytes_to_copy;

    printk(KERN_INFO "basic_char : read %zo bytes\n", bytes_to_copy);

    return bytes_to_copy;
}
static void case_conv(char *str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] ^= 32;   // toggle case
    }
}

static ssize_t basic_write(struct file *file,
                           const char __user *user_buffer,
                           size_t count, loff_t *offset)
{
    size_t bytes_to_copy;

    if (count == 0)
        return 0;

    bytes_to_copy = min(count, (size_t)(BUF_SIZE - 1));

    if (copy_from_user(kernal_buffer, user_buffer, bytes_to_copy))
        return -EFAULT;

    kernal_buffer[bytes_to_copy] = '\0';

    case_conv(kernal_buffer);
    buffer_size = bytes_to_copy;     
    *offset = 0;

    return bytes_to_copy;
}



static struct file_operations basic_fops ={
  .owner = THIS_MODULE,
  .open = basic_open,
  .read = basic_read,
  .write = basic_write,
  .release = basic_release,
};

static int __init basic_char_init(void){
  // register char device  0 -> dynamic major number
  major_number = register_chrdev(0,DEVICE_NAME,&basic_fops);
  if(major_number < 0 ){
    printk(KERN_ERR "basic_char : failed to register device\n");
    return major_number;
  }
    printk(KERN_INFO "basic_char : Loaded\n");
    printk(KERN_INFO "basic_char : Major Number: %d\n",major_number);
    printk(KERN_INFO "Create Device Node with\n");
    printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);

    return 0;
}
static void __exit basic_char_exit(void){
  unregister_chrdev(major_number,DEVICE_NAME);
  printk(KERN_INFO "Basic_char: Unloaded\n");
}

module_init(basic_char_init);
module_exit(basic_char_exit);
MODULE_AUTHOR("Rahul");
MODULE_DESCRIPTION("Educatinal Basic Character Driver with file Operations");

