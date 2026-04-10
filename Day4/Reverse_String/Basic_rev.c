#include<linux/module.h> //requ for all kernal modules
#include<linux/kernel.h> // for printk
#include<linux/init.h>  // for __int and __exit
#include<linux/fs.h>  // register_chrdev , file_opearations
#include<linux/uaccess.h>  // copy_to_user , copy_from_user

MODULE_LICENSE("GPL");

#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256

static int major_number;
static char kernal_buffer[BUF_SIZE];
static int buffer_size;
/*
 * called when user opens /dev/basic_char
 */
static int basic_open(struct inode *inode,struct file *file){
  printk(KERN_INFO "Basic kernal module opened\n");
  return 0;
}
/*
 * called when user closes /dev/basic_char
 */
static int basic_release(struct inode *inode,struct file *file){
  printk(KERN_INFO "Basic kernal module closed\n");
  return 0;
}
/*
 * called when user reads from /dev/basic_char
 */
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
static ssize_t basic_write(struct file *file, const char __user *user_buffer,
                           size_t count, loff_t *offset)
{
    size_t bytes_to_copy;
    int i;

    memset(kernal_buffer, 0, BUF_SIZE);

    bytes_to_copy = min(count, (size_t)(BUF_SIZE - 1));

    if (copy_from_user(kernal_buffer, user_buffer, bytes_to_copy))
        return -EFAULT;

    if (kernal_buffer[bytes_to_copy - 1] == '\n') {
        bytes_to_copy--;
    }

    for (i = 0; i < bytes_to_copy / 2; i++) {
        char temp = kernal_buffer[i];
        kernal_buffer[i] = kernal_buffer[bytes_to_copy - i - 1];
        kernal_buffer[bytes_to_copy - i - 1] = temp;
    }

    kernal_buffer[bytes_to_copy] = '\0';
    buffer_size = bytes_to_copy;

    *offset = 0; 

    printk(KERN_INFO "basic_char : received & reversed string : %s\n",kernal_buffer);

    return bytes_to_copy;
}
/*file operations structure
 * this connects system calls to driver functions */

static struct file_operations basic_fops ={
  .owner = THIS_MODULE,
  .open = basic_open,
  .read = basic_read,
  .write = basic_write,
  .release = basic_release,
};
//MODULE INITILIZATION
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

