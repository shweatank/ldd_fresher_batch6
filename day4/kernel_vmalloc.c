#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<init/slab.h>  //for kmalloc
#include<init/vmalloc.h>  //for vmalloc

#define KMALLOC_SIZE 1024 //1KB
#define VMALLOC_SIZE (1024*1024) //1MB

static char *kmalloc_ptr;
static char *vmalloc_ptr;
static int __init mem_demo_init(void)
{
	printk(KERN_INFO "Memory Demo module loaded\n");
	//Allocate using kmalloc
	kmalloc_ptr=kmalloc(KMALLOC_SIZE,GFP_KERNEL);
	if(!kmalloc_ptr)
	{
		printk(KERN_ERR "kmalloc failed\n");
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
	//Free memory
	if(kmalloc_ptr)
		kfree(kmalloc_ptr);
	if(vmalloc_ptr)
		vfree(vmalloc_ptr);
	printk(KERN_INFO "Memory Demo Module Unloaded\n");
}
module_init(mem_demo_init);
module_exit(mem_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("kmalloc vs vmalloc example");
