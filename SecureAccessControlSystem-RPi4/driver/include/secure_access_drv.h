/*
 * secure_access_drv.h
 *
 * Internal kernel-only definitions for the Secure Access platform driver.
 *
 * Responsibilities split across source files:
 *   sac_core_main.c - platform_device probe, character device, sysfs, ioctl,
 *                     GPIO lookup, SPI binding to ILI9225 child device.
 *   sac_irq.c       - IRQ top-half (optional), tasklet, workqueue handler that
 *                     updates TFT text + LEDs and wakes wait queue.
 *   sac_tft.c       - ILI9225 SPI drawing, font rasterization, LED helpers.
 *   sac_uart.c      - misc /dev/uart (PL011 MMIO on GPIO14/15), linked into
 *                     secure_access_core.ko; registered from sac_probe().
 * struct sac_device holds all runtime state (locks, auth snapshot, SPI handle,
 * GPIO descriptors, stats). Include order pulls in secure_access_ioctl.h for
 * sac_auth_result layout shared with userspace.
 */

#ifndef _SECURE_ACCESS_DRV_H
#define _SECURE_ACCESS_DRV_H

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/semaphore.h>
#include <linux/spi/spi.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

#include "secure_access_ioctl.h"

#define SAC_DRIVER_NAME "secure_access_ctrl"
#define SAC_CLASS_NAME  "secure_access"
#define SAC_DEV_NAME    "secure_access0"

/* sysfs stats (device_attribute stats): rolling counters for demos/monitoring. */
struct sac_stats {
	u64 auth_requests;
	u64 auth_success;
	u64 auth_failure;
	u64 auth_locked;
	u64 irq_count;
	u64 tasklet_runs;
	u64 wakeups;
};

/*
 * Per-instance driver state (one platform_device / devicetree node).
 * io_lock: serializes SPI + TFT drawing (see sac_tft.c).
 * state_lock: protects auth_res, auth_done, auth_state for ioctl vs workqueue.
 * users_sem: limits concurrent open() if extended later (currently count only).
 */
struct sac_device {
	struct device *dev;
	struct cdev cdev;
	dev_t devt;
	struct class *class;

	struct mutex io_lock;
	spinlock_t state_lock;
	struct semaphore users_sem;
	atomic_t open_count;

	wait_queue_head_t auth_wq;
	bool auth_done;
	struct sac_auth_result auth_res;
	u32 auth_state;

	int irq;
	struct tasklet_struct auth_tasklet;
	struct work_struct auth_work;
	atomic_t pending_result;

	/* Status LEDs (optional GPIOs from overlay). */
	struct gpio_desc *green_led;
	struct gpio_desc *red_led;
	/* TFT control lines (DC=data/command, RST=reset, BL=backlight optional). */
	struct gpio_desc *tft_dc;
	struct gpio_desc *tft_rst;
	struct gpio_desc *tft_bl;

	struct spi_device *tft_spi;

	struct task_struct *monitor_thread;
	struct sac_stats stats;
	/* Reserved for SAC_IOC_SET_TFT_STYLE (currently unused in drawing code). */
	u16 tft_fg_color;
	u16 tft_bg_color;
	u8 tft_scale;
};

/* TFT lifecycle / rendering */
int sac_tft_init(struct sac_device *sac);
void sac_tft_print_status(struct sac_device *sac, const char *msg);
void sac_tft_show_authenticating(struct sac_device *sac);
void sac_tft_clear(struct sac_device *sac);
void sac_tft_set_style(struct sac_device *sac, u16 fg, u16 bg, u8 scale);

/* Bottom halves + IRQ wiring */
irqreturn_t sac_irq_handler(int irq, void *dev_id);
void sac_auth_work_fn(struct work_struct *work);
void sac_tasklet_fn(unsigned long data);
void sac_fire_simulated_irq(struct sac_device *sac);

/* LED patterns (success/failure/lock) */
void sac_leds_locked_alert(struct sac_device *sac);
void sac_leds_result(struct sac_device *sac, bool ok);

/* misc /dev/uart — PL011 MMIO in secure_access_core.ko */
int sac_uart_register(void);
void sac_uart_unregister(void);

#endif
