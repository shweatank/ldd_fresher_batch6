#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#define DEVICE_NAME "dynamic_buffer"
#define IOCTL_SET_SIZE _IOW('a',1,int)

static char *buffer;
static int buffer_size;
static int major_number;

static int dynamic_buffer_open(struct inode *inode, struct file *file)
{
        return 0;
}

static int dynamic_buffer_release(struct inode *inode, struct file *file)
{
        return 0;
}

static long dynamic_buffer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        int size;

        switch(cmd)
        {
                case IOCTL_SET_SIZE:

                        if (copy_from_user(&size, (int __user *)arg, sizeof(int)))
                                return -EFAULT;

                        if (size <= 0)
                                return -EINVAL;

                        if (buffer)
                                vfree(buffer);

                        buffer = vmalloc(size);
                        if (!buffer)
                                return -ENOMEM;

                        buffer_size = size;
                        break;

                default:
                        return -EINVAL;
        }

        return 0;
}

static long dynamic_buffer_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy = min(count, (size_t)buffer_size);

        if (copy_to_user(user_buffer, buffer, bytes_to_copy))
                return -EFAULT;

        return bytes_to_copy;
}

static long dynamic_buffer_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
        if (count > buffer_size)
                return -EINVAL;

        if (copy_from_user(buffer, user_buffer, count))
                return -EFAULT;

        return count;
}

static struct file_operations fops = {
        .open = dynamic_buffer_open,
        .release = dynamic_buffer_release,
        .unlocked_ioctl = dynamic_buffer_ioctl,
        .read = dynamic_buffer_read,
        .write = dynamic_buffer_write,
};

static int __init dynamic_buffer_init(void)
{
        major_number = register_chrdev(0, DEVICE_NAME, &fops);

        if (major_number < 0)
                return major_number;

        printk(KERN_INFO "vmalloc buffer module loaded, Major = %d\n", major_number);
        return 0;
}

static void __exit dynamic_buffer_exit(void)
{
        if (buffer)
                vfree(buffer);

        unregister_chrdev(major_number, DEVICE_NAME);

        printk(KERN_INFO "vmalloc buffer module unloaded\n");
}

module_init(dynamic_buffer_init);
module_exit(dynamic_buffer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nandini");
MODULE_DESCRIPTION("Dynamic buffer module using vmalloc");
