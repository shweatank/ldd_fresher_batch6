#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>

#define DEVICE_NAME "palin_basic_char"
#define BUF_SIZE 256

static int major_number;
static char *kernel_buffer = NULL;
static int buffer_size = 0;

/* Open */
static int basic_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "palin_basic_char: device opened\n");
    return 0;
}

/* Release */
static int basic_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "palin_basic_char: device closed\n");
    return 0;
}

/* Check palindrome */
static int is_palindrome(const char *str)
{
    int i = 0;
    int j = strlen(str) - 1;

    while (i < j)
    {
        if (str[i] != str[j])
            return 0;
        i++;
        j--;
    }
    return 1;
}

/* Write */
static ssize_t basic_write(struct file *file,
                           const char __user *user_buffer,
                           size_t count,
                           loff_t *offset)
{
    char input_buf[BUF_SIZE];

    if (count >= BUF_SIZE)
        count = BUF_SIZE - 1;

    if (copy_from_user(input_buf, user_buffer, count))
        return -EFAULT;

    input_buf[count] = '\0';

    /* Remove newline if present */
    input_buf[strcspn(input_buf, "\n")] = '\0';

    /* Free old buffer */
    if (kernel_buffer)
    {
        kfree(kernel_buffer);
        kernel_buffer = NULL;
    }

    kernel_buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!kernel_buffer)
        return -ENOMEM;

    if (is_palindrome(input_buf))
        buffer_size = snprintf(kernel_buffer, BUF_SIZE, "Palindrome\n");
    else
        buffer_size = snprintf(kernel_buffer, BUF_SIZE, "Not Palindrome\n");

    printk(KERN_INFO "palin_basic_char: checked string = %s\n", input_buf);

    return count;
}

/* Read */
static ssize_t basic_read(struct file *file,
                          char __user *user_buffer,
                          size_t count,
                          loff_t *offset)
{
    size_t bytes_to_copy;

    if (!kernel_buffer || *offset >= buffer_size)
        return 0;

    bytes_to_copy = min(count, (size_t)(buffer_size - *offset));

    if (copy_to_user(user_buffer, kernel_buffer + *offset, bytes_to_copy))
        return -EFAULT;

    *offset += bytes_to_copy;

    return bytes_to_copy;
}

/* File operations */
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = basic_open,
    .read    = basic_read,
    .write   = basic_write,
    .release = basic_release,
};

/* Init */
static int __init palin_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_number < 0)
    {
        printk(KERN_ALERT "palin_basic_char: failed to register\n");
        return major_number;
    }

    printk(KERN_INFO "palin_basic_char: loaded\n");
    printk(KERN_INFO "Major number = %d\n", major_number);
    printk(KERN_INFO "mknod /dev/%s c %d 0\n", DEVICE_NAME, major_number);

    return 0;
}

/* Exit */
static void __exit palin_exit(void)
{
    if (kernel_buffer)
        kfree(kernel_buffer);

    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "palin_basic_char: unloaded\n");
}

module_init(palin_init);
module_exit(palin_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Palindrome Character Driver");
