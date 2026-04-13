/*
*It only demonstrates how code is loaded into and removed from linux kernel*/


#include<linux/module.h> //Required for all kernel modules
#include<linux/kernel.h>//Required for printk
#include<linux/init.h>//REquired for __init ad __exit macros

/* 
* __init tells the kernel: "this funcrion is only needed during module loading."
*After successful load , the memory used by this function can be freed
*/

static int __init basic_module_init(void){
printk(KERN_INFO"Basic kernel module loaded\n");
return 0;
}

/* __exit tells the kernel: "This functon is only needed during module removal."
* it will not be included if the module is built into the kernel.
*/

static void __exit basic_module_exit(void){
printk(KERN_INFO"Baisc kernel module uploaded\n");
}

/* these macros tell the kernel which functions should be called when the module is inserted and removed */

module_init(basic_module_init);
module_exit(basic_module_exit);

/* mandatory module metadata */
MODULE_LICENSE("GPL"); //PREVENTS KERNEL TAINT
MODULE_AUTHOR("VAISHNAVI");
MODULE_DESCRIPTION("Most basic linux kernel module for education");

