#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>

static struct kobject demo_kobj;
static int demo_value,sum,sub,mul;

/* kobject type */
static struct kobj_type demo_ktype = {
    .sysfs_ops = &kobj_sysfs_ops,
};

/* Show */
static ssize_t value_show(struct kobject *kobj,
                         struct kobj_attribute *attr,
                         char *buf)
{
	printk(KERN_ALERT"In value show\n");
    return sprintf(buf, "%d\n", demo_value);
}

/* Store */
static ssize_t value_store(struct kobject *kobj,
                          struct kobj_attribute *attr,
                          const char *buf, size_t count)
{
	printk(KERN_ALERT"In value store\n");
    kstrtoint(buf, 10, &demo_value);
    return count;
}

static struct kobj_attribute value_attr =
    __ATTR(value, 0664, value_show, value_store);

/* Init */
static int __init sysfs_demo_init(void)
{
    int ret;
printk(KERN_ALERT"In value init\n");
    ret = kobject_init_and_add(&demo_kobj, &demo_ktype,
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
}

/* Exit */
static void __exit sysfs_demo_exit(void)
{
    sysfs_remove_file(&demo_kobj, &value_attr.attr);
    kobject_put(&demo_kobj);
    pr_info("sysfs_demo Unloaded\n");
}

module_init(sysfs_demo_init);
module_exit(sysfs_demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple sysfs driver");
MODULE_AUTHOR("Satheesh");
