#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>        // kmalloc and kfree

#define DEVICE_NAME "u_l_and_l_u_basic_char"
#define BUF_SIZE 256

static int major_number;
static char *kernel_buffer = NULL;  // dynamically allocated
static int buffer_size = 0;

/* Convert case of each character in the string */
static void convert_case(char *str)
{
    while (*str) {
        if (*str >= 'a' && *str <= 'z') {
            *str = *str - 'a' + 'A'; // Convert lowercase to uppercase
        } else if (*str >= 'A' && *str <= 'Z') {
            *str = *str - 'A' + 'a'; // Convert uppercase to lowercase
        }
        str++;
    }
}

/* Open */
static int basic_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "u_l_and_l_u_basic_char: device opened\n");
    return 0;
}

/* Release */
static int basic_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "u_l_and_l_u_basic_char: device closed\n");
    return 0;
}

/* Write: perform case conversion */
static ssize_t basic_write(struct file *file,
                           const char __user *user_buffer,
                           size_t count,
                           loff_t *offset)
{
    int ret;

    if(count >= BUF_SIZE)
        count = BUF_SIZE - 1;

    char input_buf[BUF_SIZE];

    if(copy_from_user(input_buf, user_buffer, count))
    {
        return -EFAULT;
    }

    input_buf[count] = '\0';

    // Convert the case of the input string
    convert_case(input_buf);

    /* Allocate memory for the result */
    if(kernel_buffer)
    {
        kfree(kernel_buffer);
        kernel_buffer = NULL;
    }

    kernel_buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
    if(!kernel_buffer)
    {
        printk(KERN_ALERT "u_l_and_l_u_basic_char: kmalloc failed\n");
        buffer_size = 0;
        return -ENOMEM;
    }

    buffer_size = snprintf(kernel_buffer, BUF_SIZE, "%s\n", input_buf);
    printk(KERN_INFO "u_l_and_l_u_basic_char: converted string = %s", input_buf);

    return count;
}

/* Read: send result to user */
static ssize_t basic_read(struct file *file,
                          char __user *user_buffer,
                          size_t count,
                          loff_t *offset)
{
    size_t bytes_to_copy;

    if(!kernel_buffer || *offset >= buffer_size)
        return 0;

    bytes_to_copy = min(count, (size_t)(buffer_size - *offset));

    if(copy_to_user(user_buffer, kernel_buffer + *offset, bytes_to_copy))
        return -EFAULT;

    *offset += bytes_to_copy;
    printk(KERN_INFO "u_l_and_l_u_basic_char: read %zu bytes\n", bytes_to_copy);
    return bytes_to_copy;
}

/* File operations */
static struct file_operations basic_fops = {
    .owner   = THIS_MODULE,
    .open    = basic_open,
    .read    = basic_read,
    .write   = basic_write,
    .release = basic_release,
};

/* Init */
static int __init basic_char_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &basic_fops);

    if(major_number < 0)
    {
        printk(KERN_ALERT "u_l_and_l_u_basic_char: failed to register\n");
        return major_number;
    }

    printk(KERN_INFO "u_l_and_l_u_basic_char: loaded\n");
    printk(KERN_INFO "major number = %d\n", major_number);
    printk(KERN_INFO "Create device using:\n");
    printk(KERN_INFO "mknod /dev/%s c %d 0\n", DEVICE_NAME, major_number);

    return 0;
}

/* Exit */
static void __exit basic_char_exit(void)
{
    if(kernel_buffer)
    {
        kfree(kernel_buffer);
        kernel_buffer = NULL;
    }

    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "u_l_and_l_u_basic_char: unloaded\n");
}

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Basic Character Driver for Case Conversion");
