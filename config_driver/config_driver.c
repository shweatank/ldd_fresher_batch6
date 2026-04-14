#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "config_ioctl"

struct config_t {
    int mode;
    int speed;
    char name[32];
};

static struct config_t config, default_config = {.mode = 1,.speed = 10000,.name = "Config_driver"};

#define CONFIG_IOCTL_MAGIC 'A'
#define SET_CONFIG_ _IOWR(CONFIG_IOCTL_MAGIC, 1, struct config_t)
#define GET_CONFIG_ _IOWR(CONFIG_IOCTL_MAGIC, 2, struct config_t)
#define RESET_DEFAULT_CONFIG_ _IOWR(CONFIG_IOCTL_MAGIC, 3, struct config_t)

static int major;

//ioctl handler
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{

	switch(cmd)
	{
		case SET_CONFIG_ :
			struct config_t cfg;
			if(copy_from_user(&cfg, (int __user *)arg, sizeof(struct config_t)))
				return -EFAULT;
                        config = cfg;  
			
			break;

		case GET_CONFIG_ :
			
			if(copy_to_user( (int __user*)arg, &config, sizeof(struct config_t))){
			  return -EFAULT;
			}
			break;

		case RESET_DEFAULT_CONFIG_ :
			config = default_config;
			if(copy_to_user( (int __user*)arg, &config, sizeof(struct config_t))){
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
	pr_info("config-driver: loaded, major = %d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("config-drv:  unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");


