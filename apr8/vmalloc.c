#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h> //for kmalloc
#include <linux/vmalloc.h> //for vmalloc

#define KMALLOC_SIZE 1024 //1KB
#define VMALLOC_SIZE (1024*1024) //1MB


static char *kmalloc_ptr;
static char *vmalloc_ptr;

static int __init mem_demo_init(void)
{
	printk(KERN_INFO "Memory Demo Module Loaded\n");

	//Allocate using kmalloc 
	kmalloc_ptr=kmalloc(KMALLOC_SIZE,GFP_KERNEL);
	if(!kmalloc_ptr)
	{
		printk(KERN_ERR "kmalloc failed\n");
		return -ENOMEM;
	}
	//Allocate using vmalloc
	vmalloc_ptr=vmalloc(VMALLOC_SIZE);
	if(!vmalloc_ptr)
	{
		printk(KERN_ERR "vmalloc failed\n");
		return -ENOMEM;
	}
	//write data
	strcpy(kmalloc_ptr,"Hello from kmalloc!");
	strcpy(vmalloc_ptr,"Hello from vmalloc!");

	printk(KERN_INFO "kmalloc address:%p,content:%s\n",kmalloc_ptr,kmalloc_ptr);
	printk(KERN_INFO "vmalloc address:%p,content:%s\n",vmalloc_ptr,vmalloc_ptr);
	return 0;
}
static void __exit mem_demo_exit(void)
{
	//free memory
	if(kmalloc_ptr)
	{
		kfree(kmalloc_ptr);
	}
	if(vmalloc_ptr)
	{
		kfree(vmalloc_ptr);
	}
	printk(KERN_INFO "Memory demo Module unloaded\n");
}

module_init(mem_demo_init);
module_exit(mem_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pavani");
MODULE_DESCRIPTION("kmalloc vs vmalloc example");
