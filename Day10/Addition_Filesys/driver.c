#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>

static struct kobject *demo_kobj;

static int val1, val2;

/* Show val1 */
static ssize_t val1_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", val1);
}

/* Store val1 */
static ssize_t val1_store(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count) {
    kstrtoint(buf, 10, &val1);
    return count;
}

/* Show val2 */
static ssize_t val2_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", val2);
}

/* Store val2 */
static ssize_t val2_store(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count) {
    kstrtoint(buf, 10, &val2);
    return count;
}

/* Show result (val1 + val2) */
static ssize_t result_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", val1 + val2);
}

/* Attributes */
static struct kobj_attribute val1_attr = __ATTR(val1, 0664, val1_show, val1_store);
static struct kobj_attribute val2_attr = __ATTR(val2, 0664, val2_show, val2_store);
static struct kobj_attribute result_attr = __ATTR(result, 0444, result_show, NULL);

static int __init sysfs_demo_init(void)
{
    int ret;

    demo_kobj = kobject_create_and_add("sysfs_add", kernel_kobj);
    if (!demo_kobj)
        return -ENOMEM;

    ret = sysfs_create_file(demo_kobj, &val1_attr.attr);
    if (ret) goto err;

    ret = sysfs_create_file(demo_kobj, &val2_attr.attr);
    if (ret) goto err;

    ret = sysfs_create_file(demo_kobj, &result_attr.attr);
    if (ret) goto err;

    pr_info("sysfs_add module loaded\n");
    return 0;

err:
    kobject_put(demo_kobj);
    return ret;
}

static void __exit sysfs_demo_exit(void)
{
    kobject_put(demo_kobj);
    pr_info("sysfs_add module unloaded\n");
}

module_init(sysfs_demo_init);
module_exit(sysfs_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rahul");
MODULE_DESCRIPTION("Sysfs Addition Driver");
