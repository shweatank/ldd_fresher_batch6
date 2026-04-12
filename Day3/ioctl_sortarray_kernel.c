#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>

#define BUFSIZE 256
#define DEVICE_NAME "basic_char"

static int major_number;

struct array_data {
    int size;
    int arr[50];
};

#define MY_IOCTL_MAGIC 'k'
#define SORT_ARRAY _IOWR(MY_IOCTL_MAGIC, 1, struct array_data)

static int palindrome_open(struct inode *inode, struct file *file)
{
    printk("Device opened\n");
    return 0;
}

static int palindrome_release(struct inode *inode, struct file *file)
{
    printk("Device close\n");
    return 0;
}

static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct array_data data;
    int i, j, temp;

    switch (cmd)
    {
        case SORT_ARRAY:

            if (copy_from_user(&data, (struct array_data *)arg, sizeof(data)))
                return -EFAULT;

            for (i = 0; i < data.size - 1; i++)
            {
                for (j = i + 1; j < data.size; j++)
                {
                    if (data.arr[i] > data.arr[j])
                    {
                        temp = data.arr[i];
                        data.arr[i] = data.arr[j];
                        data.arr[j] = temp;
                    }
                }
            }

            printk(KERN_INFO "Sorted array via ioctl\n");

            if (copy_to_user((struct array_data *)arg, &data, sizeof(data)))
                return -EFAULT;

            break;

        default:
            return -EINVAL;
    }

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = palindrome_open,
    .release = palindrome_release,
    .unlocked_ioctl = basic_ioctl,
};

static int __init basic_char_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_number < 0)
    {
        printk(KERN_ERR "basic_char: failed to register device\n");
        return major_number;
    }

    printk(KERN_INFO "basic_char: loaded\n");
    printk(KERN_INFO "major number = %d\n", major_number);

    return 0;
}

static void __exit basic_char_exit(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "basic_char: unloaded\n");
}

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nandini");
MODULE_DESCRIPTION("IOCTL Array Sorting Driver");
