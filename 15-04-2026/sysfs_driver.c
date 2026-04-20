#include<linux/module.h>
#include<linux/kobject.h>
#include<linux/sysfs.h>

static struct kobject *demo_kobj;
static int demo_value;

/* Show function */

static ssize_t value_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
        return sprintf(buf,"%d\n",demo_value);
}

/* Store function */

static ssize_t value_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{
        sscanf(buf,"%d",&demo_value);
        return count;
}

static struct kobj_attribute value_attr =
    __ATTR(value,0664,value_show,value_store);

static int __init sysfs_demo_init(void)
{
        demo_kobj = kobject_create_and_add("sysfs_demo",kernel_kobj);
        if(!demo_kobj)
                return -ENOMEM;

        sysfs_create_file(demo_kobj,&value_attr.attr);
        pr_info("sysfs_demo loaded\n");
        return 0;
}

static void __exit sysfs_demo_exit(void)
{
        kobject_put(demo_kobj);
        pr_info("sysfs_demo unloaded\n");
}

module_init(sysfs_demo_init);
module_exit(sysfs_demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple sysfs driver");
MODULE_AUTHOR("Techdhaba");
