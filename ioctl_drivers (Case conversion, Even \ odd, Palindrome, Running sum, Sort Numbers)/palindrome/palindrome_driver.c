#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "palindrome_ioctl"

struct palindrome_t {
    char buf[128];
    int status;
};

static struct palindrome_t req;

#define IOCTL_MAGIC 'A'
#define PALINDROME_  _IOWR(IOCTL_MAGIC, 1, struct palindrome_t)


static int major;

static int my_strlen(char *str)
{
      int count = 0;
      while(*str)
      {
	      count++;
	      str++;
      }
      return count;
}

static int isPalindrome(char *l, char *r)
{
	while(l<r)
	{
		if(*l != *r)
			return 0;
		l++,r--;
	}
	return 1;

}

//ioctl handler
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{

	switch(cmd)
	{
		case PALINDROME_ :
			if(copy_from_user(&req, (int __user *)arg, sizeof(struct palindrome_t)))
				return -EFAULT;
			 pr_info("Palindrome-driver : received Data : %s  from user\n",req.buf);

		         req.status = isPalindrome(req.buf,req.buf + my_strlen(req.buf) - 1);	
			
			 if(copy_to_user((int __user*)arg, &req,sizeof(struct palindrome_t))){
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
	pr_info("case-conv-driver: loaded, major = %d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("case-conv-drv:  unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");


