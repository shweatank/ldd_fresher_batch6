#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/kernel.h>
MODULE_DESCRIPTION("Practicing cdev");
MODULE_AUTHOR("Satheesh");
MODULE_LICENSE("cdev");
static ssize_t my_read(struct file*file,char __user *user_buf,size_t size,loff_t* offset)
{
	return size;
}
static ssize_t my_write(struct file*file,const char __user *user_buf,size_t size,loff_t *offset)
{
	return size;
}
struct file_operations fops={.owner=THIS_MODULE,.read=my_read,.write=my_write};

static int __init my_init(void)
{
	struct cdev* my_cdev=cdev_alloc();
	my_cdev->ops=&fops;
	cdev_init(my_cdev,&fops);
	cdev_add(my_cdev,ZZ
	printk("Hello Im in Lernel\n");
	return 0;
}

static void __exit my_exit(void)
{
	printk("Exiting from kernel space\n");
}

module_init(my_init);
module_exit(my_exit);

