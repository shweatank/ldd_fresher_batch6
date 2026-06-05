/*
* it only demonstrates how code is loadex into
* and removed from the linux Kernal.
*/

#include<linux/module.h>   //Required for all kernel modules
#include<linux/kernel.h>   //Required for printk()
#include<linux/init.h>   //Required for__init and __exit macros

/*
* __init tells the kernel:
* "this function is only neeeded during module loading."
* After successful load , the memory used by this function
* can be freed.
*/

static int __init basic_module_init(void){
printk(KERN_INFO "Basic kernel module loaded\n");
return 0; //Returning 0 means successful load
}

/*
* __exit tells the kernal:
* "this function is only needed during module removal."
* it will not be included if the module is built into the kernal.
*/

static void __exit basic_module_exit(void){
printk(KERN_INFO"Basic kernal module unloaded\n");
}

/*
* these macros tell the kernek which functions
* should be called when the module is inserted and removed.
*/

module_init(basic_module_init);
module_exit(basic_module_exit);


// mandatory module metedata

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PAVAN");
MODULE_DESCRIPTION("Most basic Linux kernal module for education");
