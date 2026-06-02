#include <linux/cdev.h>
#include <linux/completion.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/random.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/sysfs.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>

#include "../include/sensor_ioctl.h"

#define PROC_NODE "sensor_monitor"
#define BUF_SZ 256
#define MMAP_PAGES 1

struct sensor_sample {
    int temperature;
    int vibration;
    int emergency;
    u64 ts_ns;
};

struct sensor_dev {
    dev_t devt;
    struct cdev cdev;
    struct class *class;
    struct device *device;

    struct sensor_sample sample;
    struct sensor_threshold threshold;
    struct sensor_stats stats;

    struct mutex state_lock;
    spinlock_t sample_lock;
    atomic_t emergency_pending;
    struct completion emergency_done;

    wait_queue_head_t read_wq;
    wait_queue_head_t event_wq;
    int data_ready;
    int monitoring;
    int buzzer;

    struct timer_list sample_timer;
    struct workqueue_struct *wq;
    struct work_struct sample_work;
    struct task_struct *sim_thread;
    struct fasync_struct *async_queue;

    struct proc_dir_entry *proc;
    struct dentry *debug_root;

    void *mmap_buf;
    size_t mmap_len;
};

static struct sensor_dev gdev;

static void generate_sample_locked(struct sensor_dev *dev)
{
    u32 r;

    get_random_bytes(&r, sizeof(r));
    dev->sample.temperature = 20 + (r % 90);
    get_random_bytes(&r, sizeof(r));
    dev->sample.vibration = r % 100;
    dev->sample.ts_ns = ktime_get_ns();
    dev->sample.emergency = atomic_read(&dev->emergency_pending);

    dev->stats.samples++;
    if (dev->sample.temperature > dev->threshold.temperature_high)
        dev->stats.temp_alerts++;
    if (dev->sample.vibration > dev->threshold.vibration_high)
        dev->stats.vib_alerts++;
}

static void push_mmap_snapshot(struct sensor_dev *dev)
{
    struct sensor_sample *dst;

    if (!dev->mmap_buf)
        return;

    dst = (struct sensor_sample *)dev->mmap_buf;
    *dst = dev->sample;
}

static void sensor_work_fn(struct work_struct *work)
{
    struct sensor_dev *dev = container_of(work, struct sensor_dev, sample_work);
    unsigned long flags;

    spin_lock_irqsave(&dev->sample_lock, flags);
    generate_sample_locked(dev);
    push_mmap_snapshot(dev);
    dev->data_ready = 1;
    spin_unlock_irqrestore(&dev->sample_lock, flags);

    wake_up_interruptible(&dev->read_wq);
    wake_up_interruptible(&dev->event_wq);
    kill_fasync(&dev->async_queue, SIGIO, POLL_IN);
}

static void sample_timer_cb(struct timer_list *t)
{
    struct sensor_dev *dev = container_of(t, struct sensor_dev, sample_timer);

    if (dev->monitoring)
        queue_work(dev->wq, &dev->sample_work);

    mod_timer(&dev->sample_timer, jiffies + msecs_to_jiffies(1000));
}

static int simulation_thread_fn(void *arg)
{
    struct sensor_dev *dev = arg;

    while (!kthread_should_stop()) {
        msleep(2500);
        if (!dev->monitoring)
            continue;

        if ((get_random_u32() % 20) == 0) {
            atomic_set(&dev->emergency_pending, 1);
            dev->stats.emergency_events++;
            wake_up_interruptible(&dev->event_wq);
            kill_fasync(&dev->async_queue, SIGIO, POLL_PRI);
            complete_all(&dev->emergency_done);
        }
    }

    return 0;
}

static ssize_t sensor_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    struct sensor_dev *dev = &gdev;
    char out[BUF_SZ];
    int n;
    unsigned long flags;

    if (len == 0)
        return 0;

    if (wait_event_interruptible(dev->read_wq, dev->data_ready))
        return -ERESTARTSYS;

    spin_lock_irqsave(&dev->sample_lock, flags);
    n = scnprintf(out, sizeof(out), "temp=%d vib=%d emergency=%d ts=%llu\n",
                  dev->sample.temperature, dev->sample.vibration,
                  atomic_read(&dev->emergency_pending),
                  dev->sample.ts_ns);
    dev->data_ready = 0;
    spin_unlock_irqrestore(&dev->sample_lock, flags);

    if (*off >= n)
        return 0;
    if (len > n - *off)
        len = n - *off;

    if (copy_to_user(buf, out + *off, len))
        return -EFAULT;
    *off += len;
    return len;
}

static ssize_t sensor_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    struct sensor_dev *dev = &gdev;
    char in[32];

    if (len >= sizeof(in))
        return -EINVAL;
    if (copy_from_user(in, buf, len))
        return -EFAULT;
    in[len] = '\0';

    if (sysfs_streq(in, "buzzer_on"))
        dev->buzzer = 1;
    else if (sysfs_streq(in, "buzzer_off"))
        dev->buzzer = 0;
    else if (sysfs_streq(in, "emergency_ack")) {
        atomic_set(&dev->emergency_pending, 0);
        reinit_completion(&dev->emergency_done);
    }

    wake_up_interruptible(&dev->event_wq);
    return len;
}

static long sensor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct sensor_dev *dev = &gdev;
    struct sensor_threshold th;
    struct sensor_stats stats;
    int bz;

    if (_IOC_TYPE(cmd) != SENSOR_IOC_MAGIC)
        return -ENOTTY;

    mutex_lock(&dev->state_lock);
    switch (cmd) {
    case SENSOR_IOCTL_START_MONITORING:
        dev->monitoring = 1;
        break;
    case SENSOR_IOCTL_STOP_MONITORING:
        dev->monitoring = 0;
        break;
    case SENSOR_IOCTL_SET_THRESHOLD:
        if (copy_from_user(&th, (void __user *)arg, sizeof(th))) {
            mutex_unlock(&dev->state_lock);
            return -EFAULT;
        }
        dev->threshold = th;
        break;
    case SENSOR_IOCTL_ENABLE_BUZZER:
        if (copy_from_user(&bz, (void __user *)arg, sizeof(bz))) {
            mutex_unlock(&dev->state_lock);
            return -EFAULT;
        }
        dev->buzzer = !!bz;
        break;
    case SENSOR_IOCTL_GET_STATS:
        stats = dev->stats;
        if (copy_to_user((void __user *)arg, &stats, sizeof(stats))) {
            mutex_unlock(&dev->state_lock);
            return -EFAULT;
        }
        break;
    default:
        mutex_unlock(&dev->state_lock);
        return -ENOTTY;
    }
    mutex_unlock(&dev->state_lock);

    wake_up_interruptible(&dev->event_wq);
    return 0;
}

static __poll_t sensor_poll(struct file *file, poll_table *wait)
{
    struct sensor_dev *dev = &gdev;
    __poll_t mask = 0;

    poll_wait(file, &dev->read_wq, wait);
    poll_wait(file, &dev->event_wq, wait);

    if (dev->data_ready)
        mask |= EPOLLIN | EPOLLRDNORM;
    if (atomic_read(&dev->emergency_pending))
        mask |= EPOLLPRI;

    return mask;
}

static int sensor_fasync(int fd, struct file *file, int on)
{
    return fasync_helper(fd, file, on, &gdev.async_queue);
}

static int sensor_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct sensor_dev *dev = &gdev;
    unsigned long pfn;
    unsigned long size = vma->vm_end - vma->vm_start;

    if (size > dev->mmap_len)
        return -EINVAL;

    pfn = vmalloc_to_pfn(dev->mmap_buf);
    if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot))
        return -EAGAIN;
    return 0;
}

static int sensor_open(struct inode *inode, struct file *file)
{
    file->private_data = &gdev;
    return 0;
}

static int sensor_release(struct inode *inode, struct file *file)
{
    sensor_fasync(-1, file, 0);
    return 0;
}

static const struct file_operations sensor_fops = {
    .owner = THIS_MODULE,
    .open = sensor_open,
    .read = sensor_read,
    .write = sensor_write,
    .unlocked_ioctl = sensor_ioctl,
    .poll = sensor_poll,
    .mmap = sensor_mmap,
    .release = sensor_release,
    .fasync = sensor_fasync,
};

static ssize_t status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct sensor_dev *s = &gdev;
    return scnprintf(buf, PAGE_SIZE, "monitoring=%d buzzer=%d emergency=%d\n",
                     s->monitoring, s->buzzer, atomic_read(&s->emergency_pending));
}

static ssize_t threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct sensor_dev *s = &gdev;
    return scnprintf(buf, PAGE_SIZE, "%d %d\n",
                     s->threshold.temperature_high, s->threshold.vibration_high);
}

static ssize_t threshold_store(struct device *dev, struct device_attribute *attr,
                               const char *buf, size_t count)
{
    struct sensor_dev *s = &gdev;
    int t, v;

    if (sscanf(buf, "%d %d", &t, &v) != 2)
        return -EINVAL;
    s->threshold.temperature_high = t;
    s->threshold.vibration_high = v;
    return count;
}

static DEVICE_ATTR_RO(status);
static DEVICE_ATTR_RW(threshold);

static ssize_t proc_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    char tmp[128];
    int n;
    struct sensor_dev *s = &gdev;

    n = scnprintf(tmp, sizeof(tmp), "temp=%d vib=%d monitoring=%d buzzer=%d\n",
                  s->sample.temperature, s->sample.vibration,
                  s->monitoring, s->buzzer);
    return simple_read_from_buffer(buf, len, off, tmp, n);
}

static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
};

static int __init sensor_init(void)
{
    int ret;
    struct sensor_dev *dev = &gdev;

    memset(dev, 0, sizeof(*dev));
    mutex_init(&dev->state_lock);
    spin_lock_init(&dev->sample_lock);
    atomic_set(&dev->emergency_pending, 0);
    init_completion(&dev->emergency_done);
    init_waitqueue_head(&dev->read_wq);
    init_waitqueue_head(&dev->event_wq);
    dev->threshold.temperature_high = 70;
    dev->threshold.vibration_high = 60;
    dev->monitoring = 1;
    dev->mmap_len = MMAP_PAGES * PAGE_SIZE;

    dev->mmap_buf = vmalloc_user(dev->mmap_len);
    if (!dev->mmap_buf)
        return -ENOMEM;

    ret = alloc_chrdev_region(&dev->devt, 0, 1, SENSOR_DEVICE_NAME);
    if (ret)
        goto err_vfree;

    cdev_init(&dev->cdev, &sensor_fops);
    ret = cdev_add(&dev->cdev, dev->devt, 1);
    if (ret)
        goto err_chrdev;

    dev->class = class_create(SENSOR_CLASS_NAME);
    if (IS_ERR(dev->class)) {
        ret = PTR_ERR(dev->class);
        goto err_cdev;
    }

    dev->device = device_create(dev->class, NULL, dev->devt, NULL, SENSOR_DEVICE_NAME);
    if (IS_ERR(dev->device)) {
        ret = PTR_ERR(dev->device);
        goto err_class;
    }

    device_create_file(dev->device, &dev_attr_status);
    device_create_file(dev->device, &dev_attr_threshold);

    dev->proc = proc_create(PROC_NODE, 0444, NULL, &proc_fops);
    dev->debug_root = debugfs_create_dir("sensor_monitor", NULL);
    if (dev->debug_root) {
        debugfs_create_u32("temperature", 0444, dev->debug_root,
                           (u32 *)&dev->sample.temperature);
        debugfs_create_u32("vibration", 0444, dev->debug_root,
                           (u32 *)&dev->sample.vibration);
    }

    dev->wq = alloc_workqueue("sensor_wq", WQ_UNBOUND, 0);
    if (!dev->wq) {
        ret = -ENOMEM;
        goto err_device;
    }
    INIT_WORK(&dev->sample_work, sensor_work_fn);
    timer_setup(&dev->sample_timer, sample_timer_cb, 0);
    mod_timer(&dev->sample_timer, jiffies + msecs_to_jiffies(1000));

    dev->sim_thread = kthread_run(simulation_thread_fn, dev, "sensor_sim_thread");
    if (IS_ERR(dev->sim_thread)) {
        ret = PTR_ERR(dev->sim_thread);
        goto err_wq;
    }

    pr_info("sensor_driver loaded major=%d minor=%d\n", MAJOR(dev->devt), MINOR(dev->devt));
    return 0;

err_wq:
    destroy_workqueue(dev->wq);
err_device:
    if (dev->debug_root)
        debugfs_remove_recursive(dev->debug_root);
    if (dev->proc)
        proc_remove(dev->proc);
    device_remove_file(dev->device, &dev_attr_status);
    device_remove_file(dev->device, &dev_attr_threshold);
    device_destroy(dev->class, dev->devt);
err_class:
    class_destroy(dev->class);
err_cdev:
    cdev_del(&dev->cdev);
err_chrdev:
    unregister_chrdev_region(dev->devt, 1);
err_vfree:
    vfree(dev->mmap_buf);
    return ret;
}

static void __exit sensor_exit(void)
{
    struct sensor_dev *dev = &gdev;

    timer_shutdown_sync(&dev->sample_timer);
    if (dev->sim_thread)
        kthread_stop(dev->sim_thread);
    if (dev->wq) {
        flush_workqueue(dev->wq);
        destroy_workqueue(dev->wq);
    }
    if (dev->debug_root)
        debugfs_remove_recursive(dev->debug_root);
    if (dev->proc)
        proc_remove(dev->proc);
    device_remove_file(dev->device, &dev_attr_status);
    device_remove_file(dev->device, &dev_attr_threshold);
    device_destroy(dev->class, dev->devt);
    class_destroy(dev->class);
    cdev_del(&dev->cdev);
    unregister_chrdev_region(dev->devt, 1);
    vfree(dev->mmap_buf);
    pr_info("sensor_driver unloaded\n");
}

module_init(sensor_init);
module_exit(sensor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Codex");
MODULE_DESCRIPTION("Character sensor monitor driver with timer/workqueue/kthread");
