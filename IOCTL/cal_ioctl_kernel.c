#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'G'

#define CALC_IOC_ADD _IOWR(IOCTL_MAGIC , 1 , struct data)
#define CALC_IOC_SUB _IOWR(IOCTL_MAGIC , 2 , struct data)
#define CALC_IOC_MUL _IOWR(IOCTL_MAGIC , 3 , struct data)
#define CALC_IOC_DIV _IOWR(IOCTL_MAGIC , 4 , struct data)
#define CALC_GET_VALUE _IOR(IOCTL_MAGIC , 5 , int)

static int major_number;

struct data {
    int num1;
    int num2;
    int result;
    char op;
};

static long basic_ioctl(struct file *file, unsigned int command, unsigned long arg)
{
    static int res = 0;
    struct data temp;



            if (copy_from_user(&temp, (struct data __user *)arg, sizeof(temp)))
                return -EFAULT;

            switch (command)
            {
                case CALC_IOC_ADD:
                    pr_info("ADD: %d %d\n", temp.num1, temp.num2);
                    res = temp.num1 + temp.num2;
                    break;

                case CALC_IOC_SUB:
                    pr_info("SUB: %d %d\n", temp.num1, temp.num2);
                    res = temp.num1 - temp.num2;
                    break;

                case CALC_IOC_MUL:
                    pr_info("MUL: %d %d\n", temp.num1, temp.num2);
                    res = temp.num1 * temp.num2;
                    break;

                case CALC_IOC_DIV:
                    pr_info("DIV: %d %d\n", temp.num1, temp.num2);
                    if (temp.num2 == 0)
                        return -EINVAL;
                    res = temp.num1 / temp.num2;
                    break;
          

        case CALC_GET_VALUE:
            if (copy_to_user((int __user *)arg, &res, sizeof(int)))
                return -EFAULT;
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = basic_ioctl,
};

static int __init basic_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    pr_info("basic_ioctl loaded, major = %d\n", major_number);
    return 0;
}

static void __exit basic_exit(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("basic_ioctl unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gangadhar");
MODULE_DESCRIPTION("IOCTL calculator driver");
