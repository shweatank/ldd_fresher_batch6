#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>

#define BUFSIZE 256
#define DEVICE_NAME "basic_char"

static int major_number;

static char kernel_buffer[BUFSIZE];
static char driver_msg[BUFSIZE];

#define MY_IOCTL_MAGIC 'k'

struct pal_data {
    char str[BUFSIZE];
    char result[32];
};

#define SET_STRING _IOW(MY_IOCTL_MAGIC, 1, struct pal_data)
#define GET_RESULT _IOR(MY_IOCTL_MAGIC, 2, struct pal_data)

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
    struct pal_data data;
    int i, j, flag;

    switch (cmd)
    {
        case SET_STRING:

            if (copy_from_user(&data, (struct pal_data *)arg, sizeof(data)))
                return -EFAULT;

            strcpy(kernel_buffer, data.str);

            i = 0;
            j = strlen(kernel_buffer) - 1;
            flag = 0;

            while (i < j)
            {
                if (kernel_buffer[i] != kernel_buffer[j])
                {
                    flag = 1;
                    break;
                }
                i++;
                j--;
            }

            if (flag == 0)
                strcpy(driver_msg, "palindrome");
            else
                strcpy(driver_msg, "not palindrome");

            printk(KERN_INFO "Checked string: %s -> %s\n",
                   kernel_buffer, driver_msg);
            break;

        case GET_RESULT:

            strcpy(data.result, driver_msg);

            if (copy_to_user((struct pal_data *)arg, &data, sizeof(data)))
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
        printk(KERN_ERR "failed\n");
        return major_number;
    }

    printk(KERN_INFO "loaded\n");
    printk(KERN_INFO "major = %d\n", major_number);
    return 0;
}

static void __exit basic_char_exit(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "unloaded\n");
}

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nandini");
MODULE_DESCRIPTION("Palindrome ioctl driver");
