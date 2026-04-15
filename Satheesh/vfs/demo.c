#include<linux/module.h>
#include<linux/kobject.h>
#include<linux/sysfs.h>
#include <linux/kernfs.h>
#include<linux/init.h>
static struct kobject *demo_kobj;
static int demo_value;
// Show function 
static ssize_t value_show(struct kobject *kobj,struct kobj_attribute *attr, char *buff)
{
	printk(KERN_ALERT"In value show\n");
	return sprintf(buff,"%d\n",demo_value);
}
// Store function 
static ssize_t value_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{
	printk(KERN_ALERT"In value store\n");
	sscanf(buf,"%d",&demo_value);
	return count;
}
static struct kobj_attribute value_attr= __ATTR(value,0664,value_show,value_store);
static int __init sysfs_demo_init(void)
{
	demo_kobj = kobj_create_and_add("sysfs_demo", kernel_kobj);
	printk(KERN_ALERT"In value init\n");
	if(!demo_kobj)
	{
		return -ENOMEM;
	}
	sysfs_create_file(demo_kobj,&value_attr.attr);
	pr_info("Sysfs_demo loaded\n");
	return 0;
}
/*static struct kobject demo_kobj; 

static int __init sysfs_demo_init(void)
{
    int ret;

    ret = kobject_init_and_add(&demo_kobj, &ktype_default,
                               kernel_kobj, "sysfs_demo");
    if (ret)
        return ret;

    ret = sysfs_create_file(&demo_kobj, &value_attr.attr);
    if (ret) {
        kobject_put(&demo_kobj);
        return ret;
    }

    pr_info("Sysfs_demo loaded\n");
    return 0;
}*/
static void __exit sysfs_demo_exit(void)
{
	kobject_put(demo_kobj);
	pr_info("sysfs_demo Unloaded\n");
}
module_init(sysfs_demo_init);
module_exit(sysfs_demo_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple sysfs driver");
MODULE_AUTHOR("Satheesh");
