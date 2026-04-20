#include<linux/module.h> //core module access
#include<linux/kernel.h> //printk()
#include<linux/init.h> //__init,__exit
#include<linux/fs.h>  //register_chrdev, file_operations
#include<linux/uaccess.h> //copy_to_user,copy_from_user

#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256

static int major_number;
static char *kernel_buffer=NULL;
static int buffer_size;

/* 
   *called when user opens /dev/basic_char
   */

static int basic_open(struct inode *inode,struct file *file)
{
        printk(KERN_INFO "basic_char: device opened\n");
        return 0;
}

/*
   *called when user closes /dev/basic_char
   */

static int basic_release(struct inode *inode,struct file *file)
{
        printk(KERN_INFO "basic_char: device close\n");
        return 0;
}

/*
   *called when user reads from /dev/basic_char
   */

static ssize_t basic_read(struct file *file,char __user *user_buffer,
                size_t count,loff_t *offset)
{
        int bytes_to_copy;

        /*
           *if offset is beyond data,return 0(EOF)
           */
        if(*offset >= buffer_size)
                return 0;
        bytes_to_copy=min(count,(size_t)(buffer_size - *offset));

        /*
           *copy data from kernel space to user space
           */

        if(copy_to_user(user_buffer,kernel_buffer + *offset,bytes_to_copy))
                return -EFAULT;

        *offset += bytes_to_copy;

        printk(KERN_INFO "basic_char: read %d bytes\n",bytes_to_copy);
       // printk(KERN_INFO "basic_char: copied to user %s\n",user_buffer);

        kfree(kernel_buffer);
        return bytes_to_copy;
}

/*
   *called when user writes to /dev/basic_char
   */

static ssize_t basic_write(struct file *file,const char __user *user_buffer,
                size_t count,loff_t *offset)
{
        int bytes_to_copy;

       bytes_to_copy=count;
       kernel_buffer=(char *)kmalloc(bytes_to_copy*sizeof(char),GFP_KERNEL);
       if(!kernel_buffer)
       {
               printk(KERN_ALERT "Memory allocation failed\n");
               return -ENOMEM;
       }

       /* 
        *COPY DATA FROM USER SPACE TO KERNEL SPACE
          */
       if(copy_from_user(kernel_buffer,user_buffer,bytes_to_copy))
               return -EFAULT;

       buffer_size=bytes_to_copy;

       printk(KERN_INFO "basic_char: wrote %d bytes\n",bytes_to_copy);
      // printk(KERN_INFO "basic_char: received string :%s\n",kernel_buffer);
       int val=0,first=0,second=0,i=0;
  
       while(kernel_buffer[i])
       {
           if(kernel_buffer[i]==',')
           {
               i++;
               while(kernel_buffer[i]!=',')
               {
                    second=second*10+(kernel_buffer[i]-'0');
                i++;
               }
               i++;
               break;
           }
           first=first*10+(kernel_buffer[i]-'0');
           i++;
       }
  
       if(kernel_buffer[i]=='A')
       {
           val=first+second;
  
       }
       else if(kernel_buffer[i]=='S')
       {
           val=first-second;
  
       }
       else if(kernel_buffer[i]=='M')
      {
          val=first*second;
       }
       else
       {
           printk(KERN_INFO "invalid operation given\n");
       }
  
       printk(KERN_INFO "value=%d\n",val);
       sprintf(kernel_buffer,"%d",val);
       return bytes_to_copy;
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
                printk(KERN_ERR "basic_char : failed to register device\n");
                return major_number;
        }


        printk(KERN_INFO "basic_char : loaded\n");
        printk(KERN_INFO "basic_char : major number =%d\n",major_number);
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
        printk(KERN_INFO "basic_char: unloaded\n");
}

/*kernel module macros*/

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prashant");
MODULE_DESCRIPTION("Educational basic character driver with file operartions");

