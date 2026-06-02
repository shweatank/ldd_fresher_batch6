/*
 * sac_irq.c
 *
 * Bottom-half processing for authentication results.
 *
 * Flow:
 *   1) User app ioctl(SAC_IOC_AUTH_RESULT) updates sac->auth_res and calls
 *      sac_fire_simulated_irq().
 *   2) If a hardware IRQ line is configured, IRQ handler runs; otherwise the
 *      tasklet is scheduled directly from sac_fire_simulated_irq().
 *   3) Tasklet schedules sac_auth_work_fn() on the system workqueue (cannot sleep
 *      in tasklet context; workqueue can msleep for TFT timing).
 *   4) sac_auth_work_fn() paints status text on ILI9225, drives LEDs, optionally
 *      delays before restoring the idle "SECURE ACCESS / READY" screen, sets
 *      auth_done, and wakes ioctl(SAC_IOC_WAIT_RESULT) waiters.
 *
 * sac_auth_result.reason is interpreted here to pick human-readable TFT strings
 * (must stay aligned with strings produced in user/secure_access_app.c).
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>

#include "include/secure_access_drv.h"

/*
 * Process one completed auth result on the workqueue (runs in process context).
 * ready_gap_ms: delay before returning to idle screen; -1 keeps current message
 * until the next auth result (multi-step prompts).
 */
void sac_auth_work_fn(struct work_struct *work)
{
	struct sac_device *sac = container_of(work, struct sac_device, auth_work);
	struct sac_auth_result res;
	char status_text[96];
	unsigned long flags;
	int ready_gap_ms = 2000;

	spin_lock_irqsave(&sac->state_lock, flags);
	res = sac->auth_res;
	sac->auth_done = true;
	spin_unlock_irqrestore(&sac->state_lock, flags);

	switch (res.state) {
	case SAC_STATE_GRANTED:
		sac_tft_print_status(sac, "ACCESS GRANTED");
		sac_leds_result(sac, true);
		/* 1s AUTHENTICATING + 2s granted/LED blink = 3s total. */
		ready_gap_ms = 0;
		break;
	case SAC_STATE_ADMIN_OK:
		if (!strcmp(res.reason, "admin verified")) {
			sac_tft_print_status(sac, "ADMIN VERIFIED");
			sac_leds_result(sac, true);
		} else if (!strcmp(res.reason, "enter new password")) {
			sac_tft_print_status(sac, "ENTER NEW PASSWORD");
		} else if (!strcmp(res.reason, "set username and password")) {
			sac_tft_print_status(sac, "SET USERNAME\nAND PASSWORD");
		} else if (!strcmp(res.reason, "enter username")) {
			sac_tft_print_status(sac, "ENTER USER NAME");
		} else {
			sac_tft_print_status(sac, "ADMIN VERIFIED");
		}
		/* No LED on follow-up prompts (e.g. set username/password, enter username). */
		/* Keep admin-step instruction visible until next operation updates LCD. */
		ready_gap_ms = -1;
		break;
	case SAC_STATE_RESET_OK:
		if (!strcmp(res.reason, "new user registered"))
			sac_tft_print_status(sac, "NEW USER REGISTERED");
		else if (!strcmp(res.reason, "user removed"))
			sac_tft_print_status(sac, "USER REMOVED");
		else if (!strcmp(res.reason, "user data printed"))
			sac_tft_print_status(sac, "USER DATA PRINTED");
		else if (!strcmp(res.reason, "audit logs printed"))
			sac_tft_print_status(sac, "AUDIT LOGS PRINTED");
		else
			sac_tft_print_status(sac, "PASSWORD RESET");
		sac_leds_result(sac, true);
		if (!strcmp(res.reason, "user data printed") || !strcmp(res.reason, "audit logs printed"))
			ready_gap_ms = 2000;
		break;
	case SAC_STATE_LOCKED:
		snprintf(status_text, sizeof(status_text), "CREDENTIALS BLOCKED %s",
			 res.reason[0] ? res.reason : "");
		sac_tft_print_status(sac, status_text);
		sac_leds_locked_alert(sac);
		break;
	case SAC_STATE_DENIED:
	default:
		if (!strcmp(res.reason, "user not registered")) {
			sac_tft_print_status(sac, "USER NOT REGISTERED");
			sac_leds_result(sac, false);
			ready_gap_ms = 0;
		} else if (!strcmp(res.reason, "admin not verified")) {
			sac_tft_print_status(sac, "ADMIN NOT VERIFIED");
			sac_leds_result(sac, false);
			ready_gap_ms = 0;
		} else if (!strcmp(res.reason, "reenter valid password")) {
			sac_tft_print_status(sac, "RE ENTER VALID PASSWORD");
			/* Stay on screen until user submits a new password attempt. */
			ready_gap_ms = -1;
		} else if (!strcmp(res.reason, "enter valid password")) {
			sac_tft_print_status(sac, "ENTER VALID PASSWORD");
			ready_gap_ms = -1;
		} else if (!strcmp(res.reason, "user not found")) {
			sac_tft_print_status(sac, "USER NOT FOUND");
			sac_leds_result(sac, false);
			ready_gap_ms = 0;
		} else if (!strcmp(res.reason, "user exists")) {
			sac_tft_print_status(sac, "USER EXISTS");
			sac_leds_result(sac, false);
			ready_gap_ms = 0;
		} else {
			sac_tft_print_status(sac, "ACCESS DENIED");
			sac_leds_result(sac, false);
			ready_gap_ms = 0;
		}
		break;
	}

	/* Visible gap before returning to idle screen. */
	if (ready_gap_ms > 0)
		msleep(ready_gap_ms);
	if (ready_gap_ms >= 0)
		sac_tft_print_status(sac, "SECURE ACCESS\nREADY");

	sac->stats.wakeups++;
	wake_up_interruptible(&sac->auth_wq);
}

/* Tasklet: IRQ-safe bridge to schedule heavy work. */
void sac_tasklet_fn(unsigned long data)
{
	struct sac_device *sac = (struct sac_device *)data;

	sac->stats.tasklet_runs++;
	schedule_work(&sac->auth_work);
}

/* Hardware IRQ top-half: count IRQs and defer all display work to tasklet. */
irqreturn_t sac_irq_handler(int irq, void *dev_id)
{
	struct sac_device *sac = dev_id;

	sac->stats.irq_count++;
	atomic_set(&sac->pending_result, 1);
	tasklet_schedule(&sac->auth_tasklet);
	return IRQ_HANDLED;
}

/*
 * When no IRQ is wired in device tree, sac_core_main still needs the same
 * update path; schedule the tasklet directly (software-only bottom half).
 */
void sac_fire_simulated_irq(struct sac_device *sac)
{
	if (sac->irq > 0)
		sac_irq_handler(sac->irq, sac);
	else
		tasklet_schedule(&sac->auth_tasklet);
}
