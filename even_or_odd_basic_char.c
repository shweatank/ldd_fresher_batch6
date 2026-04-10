#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>        // kmalloc and kfree

#define DEVICE_NAME "even_or_odd_basic_char"
#define BUF_SIZE 256

static int major_number;
static char *kernel_buffer = NULL;  // dynamically allocated
static int buffer_size = 0;

/* Function to check if a number is even or odd */
static void check_even_odd(int number, char *result)
{
    if (number % 2 == 0) {
        strcpy(result, "Even");
    } else {
        strcpy(result, "Odd");
    }
}

/* Open */
static int basic_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "even_or_odd_checker_char: device opened\n");
    return 0;
}

/* Release */
static int basic_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "even_or_odd_checker_char: device closed\n");
    return 0;
}

/* Write: perform even-odd check */
static ssize_t basic_write(struct file *file,
                           const char __user *user_buffer,
                           size_t count,
                           loff_t *offset)
{
    int number;
    char result[BUF_SIZE];

    if(count >= BUF_SIZE)
        count = BUF_SIZE - 1;

    char input_buf[BUF_SIZE];

    if(copy_from_user(input_buf, user_buffer, count))
    {
        return -EFAULT;
    }

    input_buf[count] = '\0';

    /* Parse the integer input */
    if (sscanf(input_buf, "%d", &number) != 1) {
        return -EINVAL; // Invalid input if the string can't be parsed as an integer
    }

    /* Check even or odd */
    check_even_odd(number, result);

    /* Allocate memory for the result */
    if(kernel_buffer)
    {
        kfree(kernel_buffer);
        kernel_buffer = NULL;
    }

    kernel_buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
    if(!kernel_buffer)
    {
        printk(KERN_ALERT "even_or_odd_checker_char: kmalloc failed\n");
        buffer_size = 0;
        return -ENOMEM;
    }

    buffer_size = snprintf(kernel_buffer, BUF_SIZE, "%s\n", result);
    printk(KERN_INFO "even_or_odd_checker_char: result = %s\n", result);

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
    printk(KERN_INFO "even_or_odd_checker_char: read %zu bytes\n", bytes_to_copy);
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
        printk(KERN_ALERT "even_or_odd_checker_char: failed to register\n");
        return major_number;
    }

    printk(KERN_INFO "even_or_odd_checker_char: loaded\n");
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
    printk(KERN_INFO "even_or_odd_checker_char: unloaded\n");
}

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Even Odd Checker Character Driver");
