//basic_ioctl_drv.c
//minimal ioctl exampler: send an int from user->kernel->modify->return

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>


struct ope
{
        int data1;
        int data2;
        char p;
};


#define DEVICE_NAME "basic_ioctl_drv3"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)
#define IOCTL_CAL_VALUE _IOWR(IOCTL_MAGIC,2,struct ope)
#define IOCTL_CALGET_VALUE _IOWR(IOCTL_MAGIC,3,int)


static int major;
static int kernel_value1=0;
static int kernel_value2=0;
static int kernel_value=0;
static int final_value=0;
static char c;

/*ioctl handler */

static long basic_ioctl(struct file *file,unsigned int cmd,
                unsigned long arg)
{
        int user_value;
        struct ope var;

        switch(cmd)
        {
                case IOCTL_SET_VALUE:
                        /*copy data from user */
                        if(copy_from_user(&user_value,(int __user*)arg,sizeof(int)))
                                return -EFAULT;

                        pr_info("kernel:received %d from user\n",user_value);

                        /*kerenl modifies data */
                        kernel_value=user_value+10;

                        /*copy data back to user */
                          if(copy_to_user((int __user*)arg,&kernel_value,sizeof(int)))
                                  return -EFAULT;
                          break;

                 case IOCTL_CAL_VALUE:
                        /*copy data from user */
                        if(copy_from_user(&var,(int __user*)arg,sizeof(struct ope)))
                                return -EFAULT;

                        pr_info("kernel:received 1st %d from user\n",var.data1);
                        pr_info("kernel:received 2nd %d from user\n",var.data2);
                        pr_info("kernel:received operation %c from user\n",var.p);

                        /*kerenl modifies data */
                        kernel_value1=var.data1;
                        kernel_value2=var.data2;
                        c=var.p;

                        if(c=='A')
                        {
                              final_value=kernel_value1+kernel_value2;

                        }
                        else if(c=='S')
                        {
                                final_value=kernel_value1-kernel_value2;
                                  
                        }
                        else if(c=='M')
                        {
                                final_value=kernel_value1*kernel_value2;

                        }

                        /*copy data back to user */
                          if(copy_to_user((int __user*)arg,&var,sizeof(struct ope)))
                                  return -EFAULT;
                          break;

                    case IOCTL_CALGET_VALUE:
                        /*copy data from user */
                        if(copy_from_user(&user_value,(int __user*)arg,sizeof(int)))
                                return -EFAULT;

                        pr_info("kernel:received %d from user\n",user_value);
                        pr_info("kernel:final value %d\n",final_value);

                        /*kerenl modifies data */
                        kernel_value=final_value;

                        /*copy data back to user */
                          if(copy_to_user((int __user*)arg,&kernel_value,sizeof(int)))
                                  return -EFAULT;
                          break;



                default:return -EFAULT;
        }
        return 0;
}

static struct file_operations fops = {
        .owner=THIS_MODULE,
        .unlocked_ioctl=basic_ioctl,
};

static int __init basic_init(void)
{
        major=register_chrdev(0,DEVICE_NAME,&fops);
        pr_info("basic_ioctl_drv3 loaded,major=%d\n",major);
        return 0;
}

static void __exit basic_exit(void)
{
        unregister_chrdev(major,DEVICE_NAME);
        pr_info("basic_ioctl_drv3 unloaded\n");
      
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");

