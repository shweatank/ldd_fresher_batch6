#include<linux/module.h> //requ for all kernal modules
#include<linux/kernel.h> // for printk
#include<linux/init.h>  // for __int and __exit
#include<linux/fs.h>  // register_chrdev , file_opearations
#include<linux/uaccess.h>  // copy_to_user , copy_from_user
MODULE_LICENSE("GPL");

#define DEVICE_NAME "add_char"
#define BUF_SIZE 256

static int major_number;
static int buffer_size;
static char *kernal_buffer; // Changed from array to pointer
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
#include <linux/string.h> // For strcmp
#include <linux/slab.h> // Required for kmalloc and kfree


//static char *kernal_buffer; // Changed from array to pointer

#include <linux/vmalloc.h> // Required for vmalloc/vfree

static ssize_t basic_write(struct file *file, const char __user *user_buffer,
                           size_t count, loff_t *offset)
{
    int a, b, result = 0;
    char op[10];
    char *temp_input;

    // 1. Safety check for input size
    if (count > BUF_SIZE) count = BUF_SIZE;

    // 2. Allocate temporary space for the incoming string
    temp_input = vmalloc(count + 1);
    if (!temp_input) return -ENOMEM;

    if (copy_from_user(temp_input, user_buffer, count)) {
        vfree(temp_input);
        return -EFAULT;
    }
    temp_input[count] = '\0';

    // 3. Process the math
    if (sscanf(temp_input, "%d %d %9s", &a, &b, op) == 3) {
        if (strcmp(op, "add") == 0) {
            result = a + b;
        }

        // 4. PREVENT LEAK: Free old buffer before creating new one
        if (kernal_buffer) {
            vfree(kernal_buffer);
        }

        // 5. Allocate storage for the RESULT string (e.g., "42")
        kernal_buffer = vmalloc(32); // Results won't exceed 32 chars
        if (!kernal_buffer) {
            vfree(temp_input);
            return -ENOMEM;
        }

        // 6. Print result into the persistent buffer
        buffer_size = snprintf(kernal_buffer, 32, "%d", result);
    }

    vfree(temp_input); // Clean up temporary input
    return count;
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
  unregister_chrdev(major_number, DEVICE_NAME);
  if(kernal_buffer) {
     vfree(kernal_buffer); // Use vfree for vmalloc'd memory
  }
  printk(KERN_INFO "Basic_char: Unloaded\n");
}
module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_AUTHOR("Rahul");
MODULE_DESCRIPTION("Educatinal Basic Character Driver with file Operations");

