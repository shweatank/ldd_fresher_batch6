#include<linux/module.h>
#include<linux/interrupt.h>
#include<linux/delay.h>

#define IRQ_NUM 1
#define BUFFER_SIZE 256
#define DEVICE_NAME "irq_thread_cal"

static int major_number=0;

static char buffer[256];

static irqreturn_t irq_top(int irq,void *dev_id)
{
        return IRQ_WAKE_THREAD;
}

static irqreturn_t irq_thread(int irq,void *dev_id)
{

         int val=0,first=0,second=0,i=0;

     while(buffer[i])
     {
         if(buffer[i]==',')
         {
             i++;
             while(buffer[i]!=',')
             {
                  second=second*10+(buffer[i]-'0');
              i++;
             }
             i++;
             break;
         }
         first=first*10+(buffer[i]-'0');
         i++;
     }

     if(buffer[i]=='A')
     {
         val=first+second;

     }
     else if(buffer[i]=='S')
     {
         val=first-second;

     }
     else if(buffer[i]=='M')
     {
         val=first*second;
     }
     else
     {
         printk(KERN_INFO "invalid operation given\n");
     }

     printk(KERN_INFO"value=%d\n",val);



        pr_info("Threaded IRQ handler (can sleep)\n");

        msleep(5000);
        return IRQ_HANDLED;
}

/*
   *called when user opens /dev/basic_char
   */

static int basic_open(struct inode *inode,struct file *file)
{
        printk(KERN_INFO "irq_thread_cal: device opened\n");
        return 0;
}

/*
   *called when user closes /dev/basic_char
   */

static int basic_release(struct inode *inode,struct file *file)
{
        printk(KERN_INFO "irq_thread_cal: device close\n");
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
        if(*offset >= BUFFER_SIZE)
                return 0;
        bytes_to_copy=min(count,(size_t)(BUFFER_SIZE - *offset));

        /*
           *copy data from kernel space to user space
           */

        if(copy_to_user(user_buffer,buffer + *offset,bytes_to_copy))
                return -EFAULT;

        *offset += bytes_to_copy;

        printk(KERN_INFO "irq_thread_cal: read %d bytes\n",bytes_to_copy);
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

       printk(KERN_INFO "irq_thread_cal: wrote %d bytes\n",bytes_to_copy);
      // printk(KERN_INFO "irq_thread_cal: received string :%s\n",kernel_buffer);

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


static int __init irq_thread_init(void)
{
        /*
           *register character device
           * 0 ->dynamic major number
           */

        major_number=register_chrdev(0,DEVICE_NAME,&basic_fops);
        if(major_number <0)
        {
                printk(KERN_ERR "irq_thread_cal : failed to register device\n");
                return major_number;
        }


        printk(KERN_INFO "irq_thread_cal : loaded\n");
        printk(KERN_INFO "irq_thread_cal : major number =%d\n",major_number);
        printk(KERN_INFO "create device node with:\n");
        printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);

        return request_threaded_irq(IRQ_NUM,irq_top,irq_thread,IRQF_SHARED,"irq_threaded",(void *)irq_thread);

        return 0;
}

/*
   *module cleanup
   */

static void __exit irq_thread_exit(void)
{
        unregister_chrdev(major_number,DEVICE_NAME);

        free_irq(IRQ_NUM,(void *)irq_thread);

        printk(KERN_INFO "ioctl_imp: unloaded\n");
}




module_init(irq_thread_init);
module_exit(irq_thread_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NAME");
MODULE_DESCRIPTION("Irq_thread_calculator");
