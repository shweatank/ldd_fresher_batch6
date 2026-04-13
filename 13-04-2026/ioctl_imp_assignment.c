#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>


#define IOCTL_MAGIC_NUM 'K'

#define SET_MODE _IOW(IOCTL_MAGIC_NUM,1,int)
#define GET_MODE _IOR(IOCTL_MAGIC_NUM,2,int)
#define CLEAR_BUFFER _IO(IOCTL_MAGIC_NUM,3)
#define GET_WRITE_COUNT _IOR(IOCTL_MAGIC_NUM,4,int)

static int major_number=0;
#define DEVICE_NAME "ioctl_imp_assignment"
#define BUFFER_SIZE 256

static int device_mode=0;
static int write_count=0;
static char buffer[BUFFER_SIZE];


/*
   *called when user opens /dev/ioctl_imp_assignment
   */

static int basic_open(struct inode *inode,struct file *file)
{
        printk(KERN_INFO "ioctl_imp: device opened\n");
        return 0;
}

/*
   *called when user closes /dev/ioctl_imp_assignment
   */

static int basic_release(struct inode *inode,struct file *file)
{
        printk(KERN_INFO "ioctl_imp: device close\n");
        return 0;
}

/*
   *called when user reads from /dev/ioctl_imp_assignment
   */

static ssize_t basic_read(struct file *file,char __user *user_buffer,
                size_t count,loff_t *offset)
{
        int bytes_to_copy;

        /*
           *if offset is beyond data,return 0(EOF)
           */
        if(*offset >= BUFFER_SIZE)
                return 0;
        bytes_to_copy=min(count,(size_t)(BUFFER_SIZE - *offset));

        /*
           *copy data from kernel space to user space
           */

        if(copy_to_user(user_buffer,buffer + *offset,bytes_to_copy))
                return -EFAULT;

        *offset += bytes_to_copy;

        printk(KERN_INFO "ioctl_imp: read %d bytes\n",bytes_to_copy);
        return bytes_to_copy;
}

static ssize_t basic_write(struct file *file,const char __user *user_buffer,
                size_t count,loff_t *offset)
{
        int bytes_to_copy;

       bytes_to_copy=min(count,(size_t)BUFFER_SIZE);

       /*
        *COPY DATA FROM USER SPACE TO KERNEL SPACE
          */
       if(copy_from_user(buffer,user_buffer,bytes_to_copy))
               return -EFAULT;

       write_count++;

       printk(KERN_INFO "ioctl_imp: wrote %d bytes\n",bytes_to_copy);

       return bytes_to_copy;
}


static long basic_ioctl(struct file *file,unsigned int cmd,
                unsigned long arg)
{

        switch(cmd)
        {

                 case SET_MODE:
                        /*copy data from user */
                        if(copy_from_user(&device_mode,(int __user*)arg,sizeof(int)))
                                return -EFAULT;

                          break;

                 case GET_MODE:
                        

                        /*copy data back to user */
                          if(copy_to_user((int __user*)arg,&device_mode,sizeof(int)))
                                  return -EFAULT;
                          break;


                  case CLEAR_BUFFER:
                        for(int i=0;i<BUFFER_SIZE;i++)
                        {
                                buffer[i]='\0';
                        }
                        printk(KERN_INFO "ioctl_imp:Buffer cleared successfully");
                          break;

                    case GET_WRITE_COUNT:

                        /*copy data back to user */
                          if(copy_to_user((int __user*)arg,&write_count,sizeof(int)))
                                  return -EFAULT;
                          break;



                default:return -EFAULT;
        }
        return 0;
}




/*
   *file operartions structure
   *this connects system calls to driver functions
   */


static struct file_operations basic_fops = {
        .owner=THIS_MODULE,
        .open=basic_open,
        .read=basic_read,
        .write=basic_write,
        .release=basic_release,
        .unlocked_ioctl=basic_ioctl,
};

/*
   *module initialization
   */

static int __init basic_char_init(void)
{
        /*
           *register character device
           * 0 ->dynamic major number
           */

        major_number=register_chrdev(0,DEVICE_NAME,&basic_fops);
        if(major_number <0)
        {
                printk(KERN_ERR "ioctl_imp : failed to register device\n");
                return major_number;
        }


        printk(KERN_INFO "ioctl_imp : loaded\n");
        printk(KERN_INFO "ioctl_imp : major number =%d\n",major_number);
        printk(KERN_INFO "create device node with:\n");
        printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);
        return 0;
}

/*
   *module cleanup
   */

static void __exit basic_char_exit(void)
{
        unregister_chrdev(major_number,DEVICE_NAME);
        printk(KERN_INFO "ioctl_imp: unloaded\n");
}

/*kernel module macros*/

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prashant");
MODULE_DESCRIPTION("IOCTL implemenatation with file operations");












