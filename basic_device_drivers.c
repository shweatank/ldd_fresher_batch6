#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>    

//code to demostrate how drivers get loaded into and removed from the linux kernel

/*__init tells the kernel that : 
this function is only needed during the module loading
After successful load the memory used by this function can be freed*/

static int __init basic_module_init(void)
{
    printk(KERN_INFO "Basic kernel module loaded\n");
    return 0;
}

/*
__exit tells the kernel that : 
This func is only needes during module removal
It will not be included if the module is built into the kernel
*/

static void __exit basic_module_exit(void)
{
    printk(KERN_INFO "Basic kernel module unloaded\n");
}

//these macros tell the kernel
//which func to be called when the module is inserted and removed

module_init(basic_module_init);
module_exit(basic_module_exit);

/*
Mandatory module metadata
*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SIDDARTH");
MODULE_DESCRIPTION("Basic Linux Kernel MOdule");