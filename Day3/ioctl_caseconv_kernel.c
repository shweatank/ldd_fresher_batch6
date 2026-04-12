#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256

static int major_number;

static char kernel_buffer[BUF_SIZE];
static char result_buffer[BUF_SIZE];

#define MY_IOCTL_MAGIC 'k'

struct case_data {
    char str[BUF_SIZE];
    char result[BUF_SIZE];
};

#define SET_STRING _IOW(MY_IOCTL_MAGIC, 1, struct case_data)
#define GET_RESULT _IOR(MY_IOCTL_MAGIC, 2, struct case_data)

static int basic_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int basic_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device closed\n");
    return 0;
}

static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct case_data data;
    int i;

    switch (cmd)
    {
        case SET_STRING:

            if (copy_from_user(&data, (struct case_data *)arg, sizeof(data)))
                return -EFAULT;

            strcpy(kernel_buffer, data.str);

            for (i = 0; kernel_buffer[i] != '\0'; i++)
            {
                if (kernel_buffer[i] >= 'A' && kernel_buffer[i] <= 'Z')
                    kernel_buffer[i] += 32;
                else if (kernel_buffer[i] >= 'a' && kernel_buffer[i] <= 'z')
                    kernel_buffer[i] -= 32;
            }

            strcpy(result_buffer, kernel_buffer);

            printk(KERN_INFO "Converted string: %s\n", result_buffer);
            break;

        case GET_RESULT:

            strcpy(data.result, result_buffer);

            if (copy_to_user((struct case_data *)arg, &data, sizeof(data)))
                return -EFAULT;

            break;

        default:
            return -EINVAL;
    }

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = basic_open,
    .release = basic_release,
    .unlocked_ioctl = basic_ioctl,
};

static int __init basic_char_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_number < 0)
        return major_number;

    printk(KERN_INFO "basic_char loaded\n");
    printk(KERN_INFO "major number = %d\n", major_number);

    return 0;
}

static void __exit basic_char_exit(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "basic_char unloaded\n");
}

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nandini");
MODULE_DESCRIPTION("IOCTL case conversion driver");
