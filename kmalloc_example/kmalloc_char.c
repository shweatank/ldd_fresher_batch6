#include <linux/kernel.h>
#include <linux/module.h>
//#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AUTHOR");
MODULE_DESCRIPTION("kmalloc and kfree example");

static int *ptr = NULL;

static int __init kmalloc_example_init(void)
{
	printk(KERN_INFO "kmalloc example : init\n");

	//allocate memory for 5 intergers
	ptr = (int*)kmalloc( 5 * sizeof(int),GFP_KERNEL);

	if(!ptr)
	{
		printk(KERN_ALERT "Memory Allocation failed");
		return -ENOMEM;
	}

	//initialize values
	for(int i = 0; i < 5; i++)
	{
		ptr[i] = i * 10;
		printk(KERN_INFO "ptr[%d] = %d\n",i,ptr[i]);
	}

	return 0;
}

static void __exit kmalloc_example_exit(void)
{
	printk(KERN_INFO "kmalloc example: exit \n");
	if(ptr)
	{
		kfree(ptr);
		printk(KERN_INFO "MEMORY freed\n");
	}
}

module_init(kmalloc_example_init);
module_exit(kmalloc_example_exit);

