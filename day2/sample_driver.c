#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
// #include <ctype.h>
#include<linux/fs.h>   //register_chrdev , file operations
#include<linux/uaccess.h>  //copy_to_user , copy_from_user


#define DEVICE_NAME "basic char"
#define BUFF_SIZE 256

static int major_number;
static char kernel_buffer[BUFF_SIZE];
static int buffer_size;

/*called when user opens /dev/basic_char*/

static int basic_open(struct inode *inode , struct file *file)
{
    printk(KERN_INFO "basic_char : Device opened\n");
    return 0;
}


/*called when user closes /dev/basic_char*/
static int basic_release(struct inode *inode , struct file *file)
{
    printk(KERN_INFO "basic_char : Device closed\n");
    return 0;
}

/*Called when user reads from /dev/basic_char*/
static ssize_t basic_read(struct file *file , char __user *user_buffer,size_t count , loff_t *offset)
{
    
    int bytes_to_copy;
    // char reversed_buffer[BUFF_SIZE];

    /*If offset is beyond data then return 0*/

    if(*offset >= buffer_size)
    {
        return 0;
    }

    bytes_to_copy = min(count , (size_t)(buffer_size - *offset));

    //revrese the string read from the device file before reading
    // int i;
    // for (i = 0; i < buffer_size; i++) 
    // {
    //     reversed_buffer[i] = kernel_buffer[buffer_size - 1 - i];
    // }

    /*copy data from kernel space to user pace*/

    if(copy_to_user(user_buffer , kernel_buffer + *offset , bytes_to_copy))
    {
        return -EFAULT;
    }

    *offset += bytes_to_copy;

    printk(KERN_INFO "Iam inside write (basic_read) \n");

    printk(KERN_INFO "basic_char : read %d bytes",bytes_to_copy);

    return bytes_to_copy;


}

/*Called when user writes to /dev/basic_char  */

static ssize_t basic_write(struct file *file , const char __user *user_buffer , size_t count , loff_t *offset)
{
    
    int x , y , result ;
    char op;
    int bytes_to_copy;

    bytes_to_copy = min(count , (size_t)BUFF_SIZE);

    /*copy data from user to kernel space
    */

    if(copy_from_user(kernel_buffer , user_buffer , bytes_to_copy))
        return -EFAULT;

    kernel_buffer[bytes_to_copy] = '\0';

    buffer_size = bytes_to_copy;

    if (sscanf(kernel_buffer, "%d %d %c", &x, &y, &op) == 3) {
        switch (op)
        {
            case '+':
                result = x + y;
                break;
            case '-':
                result = x - y;
                break;
            case '*':
                result = x * y;
                break;
            case '/':
                if (y != 0) 
                {
                    result = x / y;
                } 
                else 
                {
                    printk(KERN_ERR "basic_char: Division by zero error!\n");
                    buffer_size = snprintf(kernel_buffer, BUFF_SIZE, "Error: Division By Zero");
                    return count;
                }
                break;
            default:
                printk(KERN_ERR "basic_char: Unknown operator %c\n", op);
                return bytes_to_copy;
        }
        
        // Store the result back in the buffer for the next read() call
        buffer_size = snprintf(kernel_buffer, BUFF_SIZE, "%d", result);
        
    } 
    else
    {
        printk(KERN_ERR "basic_char: Failed to parse input\n");
    }

    
    printk(KERN_INFO "Iam inside write (basic_write)\n");

    printk(KERN_INFO "basic_char: wrote %d bytes, sum is %d\n", (int)count, result);

    return bytes_to_copy;
}



// File operations structure
//This connects sys calls to driver functions
static struct file_operations basic_fops = {
    .owner = THIS_MODULE,
    .open = basic_open,
    .read = basic_read,
    .write = basic_write,
    .release = basic_release,
};

/*Module init*/

static int __init basic_char_init(void)
{
    /*
    Register character deice
    0 -> dynamic major number
    */

    major_number = register_chrdev(0 , DEVICE_NAME , &basic_fops);
    if(major_number < 0)
    {
        printk(KERN_ERR "basic_char : failed to register device\n");
        return major_number;
    }

    printk(KERN_INFO "basic_char loaded\n");
    printk(KERN_INFO "basic_char : major_number = %d",major_number);
    printk(KERN_INFO "create device node with : ");
    printk(KERN_INFO "mknod /dev/%s c %d 0\n" , DEVICE_NAME , major_number);

    return 0;
}

/*module cleanup*/


static void __exit basic_char_exit(void)
{
    unregister_chrdev(major_number , DEVICE_NAME);
    printk(KERN_INFO "basic_char : unloaded\n");
}

//kernel module macros
module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SIDDARTH");
MODULE_DESCRIPTION("Educational basic device driver with file opeartions");