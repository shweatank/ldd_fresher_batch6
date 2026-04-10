#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/slab.h>//for kmalloc and kfree
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("Kmalloc and free example");
static int *ptr=NULL;
static int __init kmalloc_example__init(void)
{
	printk(KERN_INFO"Kmalloc example:init\n");
	//Allocate memory for 5 integers
	ptr=(int*)kmalloc(5*sizeo(int),GFP_KERNEL);
	if(!ptr)
	{
		printk(KERN_ALERT"Memory allocation failed\n");
		return -ENOMEM;
	}
	//initialise values
	for(int i=0;i<5;i++){
		ptr[i]=i*10;
	printk(KERN_INFO"ptr[%d]=%d\n",i,ptr[i]);
	
	}
	return 0;
}
static void __exit kmalloc_example_exit(void)
{
	printk(KERN_INFO"Kmalloc example:exit\n");
	if(ptr)
	{
		kfree(ptr);
		printk(KERN_INFO"Memory freed\n");
	}
}
module_init(kmalloc_example_init);
module_exit(kmalloc_example_exit);


