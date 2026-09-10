#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/cdev.h>
#include<linux/device.h>



#define DEVICE_NAME "sample_device"
static  dev_t dev_num;
static struct class *class;
static struct cdev my_cdev;

static struct file_operations fops={ .owner=THIS_MODULE,};
static int __init my_init(void)
{
	pr_info("Hey Im Inserted\n");
//	int ret=register_chrdev_region(&dev_num,0,1,DEVICE_NAME);
	int ret=alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);
	if(ret<0)
	{
		pr_info("Failed to allocate major and minor number\n");	
		unregister_chrdev_region(dev_num,1);
		return ret;
	}
	pr_info("Major=%d Minor=%d\n",MAJOR(dev_num),MINOR(dev_num));
	cdev_init(&my_cdev,&fops);
	cdev_add(&my_cdev,dev_num,1);
	class=class_create(DEVICE_NAME);

	return 0;
}
static void __exit my_exit(void)
{
	pr_info("Extracted\n");
	class_destroy(class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(&dev_num,1);	

}



module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("Simple cdev try");



