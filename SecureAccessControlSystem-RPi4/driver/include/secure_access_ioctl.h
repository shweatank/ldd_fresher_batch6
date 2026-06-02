/*
 * secure_access_ioctl.h
 *
 * Userspace/kernel shared definitions for the Secure Access character device.
 *
 * What lives here:
 *   - Maximum string sizes used in ioctl payloads.
 *   - Authentication state machine values (what the driver displays / counts).
 *   - Event types the user app reports when starting an auth flow (login,
 *     register, forgot password, delete).
 *   - Struct layouts passed with ioctl commands (must stay binary-compatible
 *     between user/secure_access_app.c and the driver).
 *   - SAC_IOC_* ioctl numbers; userland includes this header with the same
 *     paths as the driver include directory.
 *
 * Kernel compatibility: uses __u32/__u16/__u8 so the same header works in
 * Linux kernel and in userspace when included from driver/include/.
 */

#ifndef _SECURE_ACCESS_IOCTL_H
#define _SECURE_ACCESS_IOCTL_H

#include <linux/ioctl.h>
#include <linux/types.h>

/* Username field size in requests; keep in sync with DB storage in user app. */
#define SAC_MAX_USER_LEN 32
/* Short ASCII tag explaining result (denied reason, lock detail, etc.). */
#define SAC_MAX_REASON_LEN 64

/*
 * Values stored in sac_auth_result.state and surfaced on TFT / stats.
 * SAC_STATE_ADMIN_OK carries prompt-specific text in .reason (see sac_irq.c).
 */
enum sac_auth_state {
	SAC_STATE_IDLE = 0,
	SAC_STATE_WAITING = 1,
	SAC_STATE_GRANTED = 2,
	SAC_STATE_DENIED = 3,
	SAC_STATE_ADMIN_OK = 4,
	SAC_STATE_RESET_OK = 5,
	SAC_STATE_LOCKED = 6,
};

/* Which application flow started (sent with SAC_IOC_AUTH_REQUEST). */
enum sac_event_type {
	SAC_EVENT_LOGIN = 1,
	SAC_EVENT_REGISTER = 2,
	SAC_EVENT_FORGOT = 3,
	SAC_EVENT_DELETE = 4,
};

/* Written by user app via SAC_IOC_AUTH_REQUEST before blocking on result. */
struct sac_auth_request {
	__u32 event_type;
	char username[SAC_MAX_USER_LEN];
};

/*
 * Written by driver after sac_fire_simulated_irq processes user-supplied result.
 * User app reads it via SAC_IOC_WAIT_RESULT or poll/read helpers.
 */
struct sac_auth_result {
	__u32 success;
	__u32 state;
	char reason[SAC_MAX_REASON_LEN];
};

/* Optional TFT styling (ioctl implemented as stub in sac_tft.c today). */
struct sac_tft_style {
	__u16 fg_rgb565;
	__u16 bg_rgb565;
	__u8 scale; /* supported: 1 or 2 */
	__u8 reserved[3];
};

/*
 * Ioctl API for /dev/secure_access0
 *   AUTH_REQUEST  - begin flow; sets WAITING state.
 *   AUTH_RESULT   - user app pushes verification outcome; triggers IRQ/tasklet.
 *   GET_STATE     - read current sac_auth_state.
 *   RESET_STATE   - clear driver-side auth snapshot (debug/admin).
 *   SET_TFT_STYLE - placeholder for future ILI9225 colors/scale.
 *   WAIT_RESULT   - block until bottom-half finishes LED/TFT update.
 *   CLEAR_TFT     - blank screen (e.g. app exit).
 */
#define SAC_IOC_MAGIC 'S'
#define SAC_IOC_AUTH_REQUEST _IOW(SAC_IOC_MAGIC, 0x01, struct sac_auth_request)
#define SAC_IOC_AUTH_RESULT _IOW(SAC_IOC_MAGIC, 0x02, struct sac_auth_result)
#define SAC_IOC_GET_STATE _IOR(SAC_IOC_MAGIC, 0x03, __u32)
#define SAC_IOC_RESET_STATE _IO(SAC_IOC_MAGIC, 0x04)
#define SAC_IOC_SET_TFT_STYLE _IOW(SAC_IOC_MAGIC, 0x05, struct sac_tft_style)
#define SAC_IOC_WAIT_RESULT _IOR(SAC_IOC_MAGIC, 0x06, struct sac_auth_result)
#define SAC_IOC_CLEAR_TFT _IO(SAC_IOC_MAGIC, 0x07)

#endif
