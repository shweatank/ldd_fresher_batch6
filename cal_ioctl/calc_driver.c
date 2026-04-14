
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "cal_ioctl"

struct CALC {
        int a;
        int b;
        long long int res;
};

static struct CALC cal_req;

#define IOCTL_MAGIC 'A'
#define CALC_IOC_ADD _IOWR(IOCTL_MAGIC, 1, struct CALC)
#define CALC_IOC_SUB _IOWR(IOCTL_MAGIC, 2, struct CALC)
#define CALC_IOC_MUL _IOWR(IOCTL_MAGIC, 3, struct CALC)
#define CALC_IOC_DIV _IOWR(IOCTL_MAGIC, 4, struct CALC)

#define CALC_GET_RES _IOWR(IOCTL_MAGIC, 5, int)


static int major;

//ioctl handler
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{

	switch(cmd)
	{
		case CALC_IOC_ADD:
			if(copy_from_user(&cal_req,(int __user *)arg, sizeof(struct CALC)))
				return -EFAULT;
			pr_info("Kernel : received Data a=%d b=%d from user\n",cal_req.a,cal_req.b);

			//kernel modifies data
			cal_req.res = cal_req.a + cal_req.b;
			//copy data back to user
			if(copy_to_user((int __user*)arg, &cal_req,sizeof(struct CALC))){
			  return -EFAULT;
			}
			break;
		case CALC_IOC_SUB:
			if(copy_from_user(&cal_req,(int __user *)arg, sizeof(struct CALC)))
                                return -EFAULT;
             //           pr_info("Kernel : received %d from user\n",user_value);

                        //kernel modifies data
                        cal_req.res = cal_req.a - cal_req.b;
                        //copy data back to user
                        if(copy_to_user((int __user*)arg, &cal_req,sizeof(struct CALC))){
                          return -EFAULT;
                        }
                        break;
                case CALC_IOC_MUL:
			if(copy_from_user(&cal_req,(int __user *)arg, sizeof(struct CALC)))
                                return -EFAULT;
               //         pr_info("Kernel : received %d from user\n",user_value);

                        //kernel modifies data
                        cal_req.res = cal_req.a * cal_req.b;
                        //copy data back to user
                        if(copy_to_user((int __user*)arg, &cal_req,sizeof(struct CALC))){
                          return -EFAULT;
                        }
                        break;
                case CALC_IOC_DIV:
			if(copy_from_user(&cal_req,(int __user *)arg, sizeof(struct CALC)))
                                return -EFAULT;
                 //       pr_info("Kernel : received %d from user\n",user_value);

                        //kernel modifies data
			if(cal_req.b == 0) 
				cal_req.res = 0;
			else
                                cal_req.res = cal_req.a / cal_req.b;
                        //copy data back to user
                        if(copy_to_user((int __user*)arg, &cal_req,sizeof(struct CALC))){
                          return -EFAULT;
                        }
                        break;
		case CALC_GET_RES:	
                        if(copy_to_user((int __user*)arg, &cal_req.res,sizeof(int))){
                          return -EFAULT;
                        }
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
	major = register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("basic_ioctl loaded, major = %d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("basic_ioctl unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");


