#include<linux/init.h>
#include<linux/module.h>
static int __init my_init(void)
{

	pr_info("Before illegal instruction\n");
	volatile int num1;
	volatile int num2=10,num=0;
	num1=num2/num;
	pr_info("After illegal instruction\n");
	return 0;
}
static void __exit my_exit(void)
{
	pr_info("Module unloaded\n");
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satheesh");
MODULE_DESCRIPTION("Testing");


