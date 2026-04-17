/*It only demonstrates how code is loaded into
 * and removed from the linux kernel
 */


#include<linux/module.h>  //required for all kernel modules
#include<linux/kernel.h>  //required for printk (print in kernel)
#include<linux/init.h>    //required for __init and __exit macros

//Mandatory module metadata
MODULE_LICENSE("GPL");  //prevents kernel taint
MODULE_AUTHOR("Satish");
MODULE_DESCRIPTION("Most basic Linux kernel module for education");

/*__init tells the kernel 
 * "This function is only needed during module loading."
 * After succeessful load,the memory used by this function 
 * can be freed.
 */
static int __init basic_module_init(void)
{
	printk(KERN_INFO "Basic kernel module loaded\n");  //print int log files       //here object is kernel object(.ko)
	return 0; //returning 0 means successful load
}
/*
 * __exit tells the kernel:
 * "This function is only needed during module removal."
 * It will not be included if the module is built into the kernel.
 */
static void __exit basic_module_exit(void)
{
	printk(KERN_INFO "Basic kernel module unloaded\n");
}
/*
 * These are macros tell the kernel which functions should be called 
 * whem the module is inserted and removed
 */
module_init(basic_module_init);
module_exit(basic_module_exit);


