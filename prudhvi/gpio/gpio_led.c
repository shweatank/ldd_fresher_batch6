#include<linux/module.h>
#include<linux/init.h>
#include<linux/gpio.h>
#include<linux/kernel.h>
#define GPIO_LED 
MODULE_LICENSE("GPL");



static int __init my_init(void)
{
pr_info("LED:installing led driver\n");
//requesting for gpio
if(!gpio_is valid(


return 0;
}


static void __exit my_exit(void)
{

return ;
}


module_init(my_init);
module_exit(my_exit);

