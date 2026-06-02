#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>

#include "include/secure_access_drv.h"

/*
 * sac_core_main.c
 *
 * Core of the out-of-tree platform driver: character device, file operations,
 * ioctl interface, and platform driver probe/remove for compatible "secure,sac-ctrl".
 *
 * Data path to hardware:
 *   - GPIO via devm_gpiod_get_optional() (LEDs, TFT DC/RST/BL).
 *   - SPI: finds child SPI device from overlay phandle "tft-spi" and uses
 *     spi_write() in sac_tft.c.
 *   - Optional platform IRQ for auth completion (or software tasklet only).
 *
 * User space protocol (see user/secure_access_app.c):
 *   open -> ioctl AUTH_REQUEST (sets WAITING) -> write "auth_wait" may show
 *   AUTHENTICATING on TFT -> user validates -> ioctl AUTH_RESULT -> bottom half
 *   in sac_irq.c -> ioctl WAIT_RESULT to synchronize.
 */

/* sysfs: single read-only "stats" attribute on the platform device. */
static ssize_t stats_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct sac_device *sac = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE,
			 "requests=%llu success=%llu failure=%llu locked=%llu irq=%llu tasklet=%llu wakeups=%llu open=%d state=%u\n",
			 sac->stats.auth_requests, sac->stats.auth_success,
			 sac->stats.auth_failure, sac->stats.auth_locked,
			 sac->stats.irq_count, sac->stats.tasklet_runs,
			 sac->stats.wakeups, atomic_read(&sac->open_count),
			 sac->auth_state);
}

static DEVICE_ATTR_RO(stats);

/* Background kernel thread; periodic debug print only (optional tuning aid). */
static int sac_monitor_thread_fn(void *data)
{
	struct sac_device *sac = data;

	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(HZ * 5);
		if (kthread_should_stop())
			break;

		dev_dbg(sac->dev,
			"monitor: state=%u pending=%d open=%d req=%llu\n",
			sac->auth_state, atomic_read(&sac->pending_result),
			atomic_read(&sac->open_count), sac->stats.auth_requests);
	}

	__set_current_state(TASK_RUNNING);
	return 0;
}

/* Character device file_operations */

static int sac_open(struct inode *inode, struct file *filp)
{
	struct sac_device *sac = container_of(inode->i_cdev, struct sac_device, cdev);
	filp->private_data = sac;

	if (down_interruptible(&sac->users_sem))
		return -ERESTARTSYS;
	atomic_inc(&sac->open_count);
	up(&sac->users_sem);
	return 0;
}

static int sac_release(struct inode *inode, struct file *filp)
{
	struct sac_device *sac = filp->private_data;

	if (down_interruptible(&sac->users_sem))
		return -ERESTARTSYS;
	atomic_dec(&sac->open_count);
	up(&sac->users_sem);
	return 0;
}

/* Read last sac_auth_result snapshot as ASCII (debug/helper interface). */

static ssize_t sac_read(struct file *filp, char __user *ubuf, size_t len, loff_t *off)
{
	struct sac_device *sac = filp->private_data;
	char kbuf[192];
	int n;

	n = scnprintf(kbuf, sizeof(kbuf), "state=%u success=%u reason=%s\n",
		      sac->auth_res.state, sac->auth_res.success, sac->auth_res.reason);
	return simple_read_from_buffer(ubuf, len, off, kbuf, n);
}

/*
 * Legacy hook: user writes "auth_wait" after AUTH_REQUEST ioctl to display
 * AUTHENTICATING... briefly (see notify_driver_waiting() in user app).
 */
static ssize_t sac_write(struct file *filp, const char __user *ubuf, size_t len, loff_t *off)
{
	struct sac_device *sac = filp->private_data;
	char kbuf[64];

	len = min_t(size_t, len, sizeof(kbuf) - 1);
	if (copy_from_user(kbuf, ubuf, len))
		return -EFAULT;
	kbuf[len] = '\0';

	if (strnstr(kbuf, "auth_wait", len)) {
		sac_tft_show_authenticating(sac);
		/* Keep AUTHENTICATING visible for 1s max. */
		msleep(1000);
	}

	return len;
}

/* Wake poll/select when sac_auth_done becomes true after bottom-half runs. */

static __poll_t sac_poll(struct file *filp, poll_table *wait)
{
	struct sac_device *sac = filp->private_data;
	__poll_t mask = 0;

	poll_wait(filp, &sac->auth_wq, wait);
	if (READ_ONCE(sac->auth_done))
		mask |= EPOLLIN | EPOLLRDNORM;
	return mask;
}

/* ioctl: primary control surface for user application. */
static long sac_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct sac_device *sac = filp->private_data;
	unsigned long flags;

	switch (cmd) {
	case SAC_IOC_AUTH_REQUEST: {
		struct sac_auth_request req;

		if (copy_from_user(&req, (void __user *)arg, sizeof(req)))
			return -EFAULT;

		spin_lock_irqsave(&sac->state_lock, flags);
		sac->auth_done = false;
		sac->auth_state = SAC_STATE_WAITING;
		memset(&sac->auth_res, 0, sizeof(sac->auth_res));
		strscpy(sac->auth_res.reason, "in progress", sizeof(sac->auth_res.reason));
		spin_unlock_irqrestore(&sac->state_lock, flags);

		sac->stats.auth_requests++;
		break;
	}
	case SAC_IOC_AUTH_RESULT: {
		struct sac_auth_result res;

		if (copy_from_user(&res, (void __user *)arg, sizeof(res)))
			return -EFAULT;

		spin_lock_irqsave(&sac->state_lock, flags);
		sac->auth_res = res;
		sac->auth_state = res.state;
		spin_unlock_irqrestore(&sac->state_lock, flags);

		if (res.state == SAC_STATE_GRANTED || res.state == SAC_STATE_ADMIN_OK ||
		    res.state == SAC_STATE_RESET_OK)
			sac->stats.auth_success++;
		else if (res.state == SAC_STATE_LOCKED)
			sac->stats.auth_locked++;
		else
			sac->stats.auth_failure++;

		sac_fire_simulated_irq(sac);
		break;
	}
	case SAC_IOC_GET_STATE: {
		u32 st = READ_ONCE(sac->auth_state);
		if (copy_to_user((void __user *)arg, &st, sizeof(st)))
			return -EFAULT;
		break;
	}
	case SAC_IOC_RESET_STATE:
		spin_lock_irqsave(&sac->state_lock, flags);
		sac->auth_done = false;
		sac->auth_state = SAC_STATE_IDLE;
		memset(&sac->auth_res, 0, sizeof(sac->auth_res));
		spin_unlock_irqrestore(&sac->state_lock, flags);
		break;
	case SAC_IOC_SET_TFT_STYLE: {
		struct sac_tft_style style;

		if (copy_from_user(&style, (void __user *)arg, sizeof(style)))
			return -EFAULT;
		sac_tft_set_style(sac, style.fg_rgb565, style.bg_rgb565, style.scale);
		break;
	}
	case SAC_IOC_CLEAR_TFT:
		sac_tft_clear(sac);
		break;
	case SAC_IOC_WAIT_RESULT: {
		int ret;
		struct sac_auth_result out;

		ret = wait_event_interruptible(sac->auth_wq, READ_ONCE(sac->auth_done));
		if (ret)
			return ret;

		out = sac->auth_res;
		if (copy_to_user((void __user *)arg, &out, sizeof(out)))
			return -EFAULT;
		break;
	}
	default:
		return -ENOTTY;
	}

	return 0;
}

static const struct file_operations sac_fops = {
	.owner = THIS_MODULE,
	.open = sac_open,
	.read = sac_read,
	.write = sac_write,
	.unlocked_ioctl = sac_ioctl,
	.release = sac_release,
	.poll = sac_poll,
	.llseek = noop_llseek,
};

/* Parse GPIO descriptors named in device tree (green, red, dc, reset, backlight). */

static int sac_parse_gpios(struct sac_device *sac)
{
	sac->green_led = devm_gpiod_get_optional(sac->dev, "green", GPIOD_OUT_LOW);
	if (IS_ERR(sac->green_led))
		return PTR_ERR(sac->green_led);
	sac->red_led = devm_gpiod_get_optional(sac->dev, "red", GPIOD_OUT_LOW);
	if (IS_ERR(sac->red_led))
		return PTR_ERR(sac->red_led);
	sac->tft_dc = devm_gpiod_get_optional(sac->dev, "dc", GPIOD_OUT_LOW);
	if (IS_ERR(sac->tft_dc))
		return PTR_ERR(sac->tft_dc);
	sac->tft_rst = devm_gpiod_get_optional(sac->dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(sac->tft_rst))
		return PTR_ERR(sac->tft_rst);
	sac->tft_bl = devm_gpiod_get_optional(sac->dev, "backlight", GPIOD_OUT_LOW);
	if (IS_ERR(sac->tft_bl))
		return PTR_ERR(sac->tft_bl);

	return 0;
}

/* Resolve tft-spi phandle to struct spi_device for ILI9225 transfers. */

static int sac_bind_spi(struct sac_device *sac)
{
	struct device_node *np;
	struct spi_device *spi;
	struct device *spi_dev;

	np = of_parse_phandle(sac->dev->of_node, "tft-spi", 0);
	if (!np) {
		dev_warn(sac->dev, "no tft-spi phandle, TFT disabled\n");
		return 0;
	}

	spi_dev = bus_find_device_by_of_node(&spi_bus_type, np);
	of_node_put(np);
	if (!spi_dev) {
		dev_warn(sac->dev, "SPI device not ready yet\n");
		return -EPROBE_DEFER;
	}
	spi = to_spi_device(spi_dev);

	sac->tft_spi = spi;
	spi->mode = SPI_MODE_0;
	spi->bits_per_word = 8;
	spi->max_speed_hz = 10000000;
	return spi_setup(spi);
}

/* Allocate major/minor, register cdev, create class device node /dev/secure_access0 */

static int sac_char_device_init(struct sac_device *sac)
{
	int ret;

	ret = alloc_chrdev_region(&sac->devt, 0, 1, SAC_DRIVER_NAME);
	if (ret)
		return ret;

	cdev_init(&sac->cdev, &sac_fops);
	sac->cdev.owner = THIS_MODULE;
	ret = cdev_add(&sac->cdev, sac->devt, 1);
	if (ret)
		goto err_unreg;

	sac->class = class_create(SAC_CLASS_NAME);
	if (IS_ERR(sac->class)) {
		ret = PTR_ERR(sac->class);
		goto err_cdev;
	}

	if (!device_create(sac->class, NULL, sac->devt, NULL, SAC_DEV_NAME)) {
		ret = -ENODEV;
		goto err_class;
	}

	return 0;

err_class:
	class_destroy(sac->class);
err_cdev:
	cdev_del(&sac->cdev);
err_unreg:
	unregister_chrdev_region(sac->devt, 1);
	return ret;
}

static void sac_char_device_deinit(struct sac_device *sac)
{
	device_destroy(sac->class, sac->devt);
	class_destroy(sac->class);
	cdev_del(&sac->cdev);
	unregister_chrdev_region(sac->devt, 1);
}

/* Platform driver: probe ordering ties together SPI, IRQ, sysfs, TFT splash. */

static int sac_probe(struct platform_device *pdev)
{
	struct sac_device *sac;
	int ret;

	sac = devm_kzalloc(&pdev->dev, sizeof(*sac), GFP_KERNEL);
	if (!sac)
		return -ENOMEM;

	sac->dev = &pdev->dev;
	mutex_init(&sac->io_lock);
	spin_lock_init(&sac->state_lock);
	sema_init(&sac->users_sem, 1);
	atomic_set(&sac->open_count, 0);
	atomic_set(&sac->pending_result, 0);
	init_waitqueue_head(&sac->auth_wq);
	tasklet_init(&sac->auth_tasklet, sac_tasklet_fn, (unsigned long)sac);
	INIT_WORK(&sac->auth_work, sac_auth_work_fn);

	ret = sac_parse_gpios(sac);
	if (ret)
		return ret;

	ret = sac_bind_spi(sac);
	if (ret)
		return ret;

	sac->irq = platform_get_irq_optional(pdev, 0);
	if (sac->irq > 0) {
		ret = devm_request_irq(&pdev->dev, sac->irq, sac_irq_handler,
				       IRQF_TRIGGER_RISING, SAC_DRIVER_NAME, sac);
		if (ret)
			return ret;
	}

	ret = sac_char_device_init(sac);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, sac);
	dev_set_drvdata(&pdev->dev, sac);

	ret = device_create_file(&pdev->dev, &dev_attr_stats);
	if (ret)
		dev_warn(&pdev->dev, "stats sysfs creation failed: %d\n", ret);

	ret = sac_tft_init(sac);
	if (!ret)
		sac_tft_print_status(sac, "SECURE ACCESS\nREADY");

	sac->monitor_thread = kthread_run(sac_monitor_thread_fn, sac, "sac_mon");
	if (IS_ERR(sac->monitor_thread)) {
		dev_warn(&pdev->dev, "monitor thread start failed\n");
		sac->monitor_thread = NULL;
	}

	ret = sac_uart_register();
	if (ret)
		dev_warn(&pdev->dev, "sac_uart misc (/dev/uart) failed: %d (audit export may use TTY fallback)\n", ret);

	dev_info(&pdev->dev, "secure access controller probed\n");
	return 0;
}

/* Reverse probe: tear down sysfs, workqueues, cdev, SPI reference. */

static void sac_remove(struct platform_device *pdev)
{
	struct sac_device *sac = platform_get_drvdata(pdev);

	sac_uart_unregister();
	if (sac->monitor_thread)
		kthread_stop(sac->monitor_thread);
	device_remove_file(&pdev->dev, &dev_attr_stats);
	tasklet_kill(&sac->auth_tasklet);
	cancel_work_sync(&sac->auth_work);
	sac_char_device_deinit(sac);
	if (sac->tft_spi)
		put_device(&sac->tft_spi->dev);
}

/* Module glue: bind driver to devices with compatible "secure,sac-ctrl". */

static const struct of_device_id sac_of_match[] = {
	{ .compatible = "secure,sac-ctrl" },
	{ }
};
MODULE_DEVICE_TABLE(of, sac_of_match);

static struct platform_driver sac_platform_driver = {
	.driver = {
		.name = SAC_DRIVER_NAME,
		.of_match_table = sac_of_match,
	},
	.probe = sac_probe,
	.remove = sac_remove,
};

module_platform_driver(sac_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Secure Access Project");
MODULE_DESCRIPTION("Secure Access Controller - modular platform driver");
