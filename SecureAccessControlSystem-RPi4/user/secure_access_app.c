#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <termios.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "../driver/include/secure_access_ioctl.h"

/*
 * secure_access_app.c
 *
 * Userspace companion for the secure_access_ctrl kernel module.
 *
 * Responsibilities:
 *   - Maintain a simple text-file credential database on removable storage
 *     (default mount point DB_ROOT below): salted SHA-256 password hashes,
 *     recovery hashes, lockout timestamps, admin flag.
 *   - Present a terminal menu for login, registration (admin-gated), forgot
 *     password, delete user, print users, exit.
 *   - For each security-sensitive step, coordinate with the driver so the TFT
 *     shows matching status strings (ioctl AUTH_REQUEST / AUTH_RESULT / WAIT).
 *
 * Security notes:
 *   - Plain secrets never logged; only hashes stored.
 *   - File permissions set to 0600 on DB files after creation.
 *   - Strong password policy enforced in software before hashing.
 *
 * Build: see user/Makefile (-lcrypto for OpenSSL EVP SHA256).
 */

/* ---------- Paths & tuning constants ---------- */

#define DEV_NODE "/dev/secure_access0"
#define DB_ROOT "/mnt/sdcard/secure_access"
#define USER_DB DB_ROOT "/users.db"
#define AUDIT_DB DB_ROOT "/audit.log"

#define MAX_LINE 512
#define SALT_HEX_LEN 64
#define HASH_HEX_LEN 64
#define MAX_ATTEMPTS 3
#define LOCK_SECONDS 60

/*
 * One line in users.db (pipe-separated fields). Must match parse/format helpers.
 * lock_until stores Unix time when account unlocks after brute-force lockout.
 */
typedef struct {
	char username[SAC_MAX_USER_LEN];
	char salt_hex[SALT_HEX_LEN + 1];
	char pass_hash[HASH_HEX_LEN + 1];
	char rec_hash[HASH_HEX_LEN + 1];
	int attempts;
	time_t lock_until;
	int is_admin;
} user_rec_t;

/* ---------- Small CLI helpers ---------- */

static void trim_newline(char *s)
{
	size_t n;
	if (!s)
		return;
	n = strlen(s);
	if (n && s[n - 1] == '\n')
		s[n - 1] = '\0';
}

static int read_line(const char *prompt, char *buf, size_t sz)
{
	printf("%s", prompt);
	fflush(stdout);
	if (!fgets(buf, (int)sz, stdin))
		return -1;
	trim_newline(buf);
	return 0;
}

/* ---------- Cryptographic primitives (OpenSSL EVP SHA-256) ---------- */

static int hex_encode(const uint8_t *in, size_t len, char *out, size_t out_sz)
{
	static const char h[] = "0123456789abcdef";
	size_t i;
	if (out_sz < (len * 2 + 1))
		return -1;
	for (i = 0; i < len; ++i) {
		out[i * 2] = h[(in[i] >> 4) & 0xF];
		out[i * 2 + 1] = h[in[i] & 0xF];
	}
	out[len * 2] = '\0';
	return 0;
}

static int sha256_salted(const char *salt_hex, const char *input, char *out_hex)
{
	EVP_MD_CTX *ctx = NULL;
	unsigned char md[EVP_MAX_MD_SIZE];
	unsigned int mdlen = 0;
	int rc = -1;

	ctx = EVP_MD_CTX_new();
	if (!ctx)
		return -1;

	if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1)
		goto out;
	if (EVP_DigestUpdate(ctx, salt_hex, strlen(salt_hex)) != 1)
		goto out;
	if (EVP_DigestUpdate(ctx, input, strlen(input)) != 1)
		goto out;
	if (EVP_DigestFinal_ex(ctx, md, &mdlen) != 1)
		goto out;

	if (hex_encode(md, mdlen, out_hex, HASH_HEX_LEN + 1) != 0)
		goto out;

	rc = 0;
out:
	EVP_MD_CTX_free(ctx);
	return rc;
}

static int generate_salt(char *salt_hex)
{
	uint8_t salt[32];
	if (RAND_bytes(salt, sizeof(salt)) != 1)
		return -1;
	return hex_encode(salt, sizeof(salt), salt_hex, SALT_HEX_LEN + 1);
}

/* ---------- Filesystem persistence (SD card directory + flat files) ---------- */

static int ensure_storage(void)
{
	FILE *fp;
	struct stat st;

	if (stat(DB_ROOT, &st) < 0) {
		if (mkdir(DB_ROOT, 0700) < 0)
			return -1;
	}

	fp = fopen(USER_DB, "a+");
	if (!fp)
		return -1;
	fclose(fp);

	fp = fopen(AUDIT_DB, "a+");
	if (!fp)
		return -1;
	fclose(fp);

	chmod(USER_DB, 0600);
	chmod(AUDIT_DB, 0600);
	return 0;
}

static int audit_line_time(const char *line, time_t *tp);

/* Rolling window: only AUDIT_RETAIN_DAYS of audit.log are kept on each new event. */
#define AUDIT_RETAIN_DAYS 2
#define AUDIT_RETAIN_SEC ((time_t)((AUDIT_RETAIN_DAYS) * 86400))

/*
 * Rewrite audit.log: drop lines older than now_sec - AUDIT_RETAIN_SEC, then append
 * the new record. Unparseable lines are kept. On failure, fall back to plain append.
 */
static int audit_rewrite_retaining(time_t now_sec, const char *wall, const char *ev,
				   const char *user, const char *status)
{
	FILE *in, *out;
	char line[MAX_LINE];
	char tmp_path[sizeof(AUDIT_DB) + 16];
	time_t cutoff = now_sec - AUDIT_RETAIN_SEC;

	if (snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", AUDIT_DB) >= (int)sizeof(tmp_path))
		return -1;

	unlink(tmp_path);
	out = fopen(tmp_path, "w");
	if (!out)
		return -1;

	in = fopen(AUDIT_DB, "r");
	if (in) {
		while (fgets(line, sizeof(line), in)) {
			time_t t;

			if (audit_line_time(line, &t) == 0) {
				if (t < cutoff)
					continue;
			}
			if (fputs(line, out) == EOF) {
				fclose(in);
				fclose(out);
				unlink(tmp_path);
				return -1;
			}
		}
		fclose(in);
	}

	if (fprintf(out, "%s|%s|%s|%s\n", wall, ev, user, status) < 0) {
		fclose(out);
		unlink(tmp_path);
		return -1;
	}
	if (fclose(out) != 0) {
		unlink(tmp_path);
		return -1;
	}
	if (rename(tmp_path, AUDIT_DB) != 0) {
		unlink(tmp_path);
		return -1;
	}
	chmod(AUDIT_DB, 0600);
	return 0;
}

/* Audit table on SD; pruned to AUDIT_RETAIN_DAYS on each append. */

static void audit_log(const char *ev, const char *user, const char *status)
{
	FILE *fp;
	struct timespec tv;
	time_t sec;
	struct tm tmv;
	char wall[64];

	/*
	 * Local wall time (tzset + localtime_r). Retention uses the same clock as
	 * audit_line_time() (mktime / epoch) so cutoff matches stored lines.
	 */
	tzset();
	if (clock_gettime(CLOCK_REALTIME, &tv) != 0)
		sec = time(NULL);
	else
		sec = tv.tv_sec;

	if (!localtime_r(&sec, &tmv))
		return;
	if (strftime(wall, sizeof(wall), "%Y-%m-%d %H:%M:%S", &tmv) == 0)
		return;

	if (audit_rewrite_retaining(sec, wall, ev, user, status) == 0)
		return;

	fp = fopen(AUDIT_DB, "a");
	if (!fp)
		return;
	fprintf(fp, "%s|%s|%s|%s\n", wall, ev, user, status);
	fclose(fp);
}

/* Serialize/deserialize user rows; rewrite_users uses atomic rename via ".tmp". */

static int parse_user_line(const char *line, user_rec_t *u)
{
	char lock_buf[32] = {0};
	char tmp[MAX_LINE];
	int n;

	memset(u, 0, sizeof(*u));
	strncpy(tmp, line, sizeof(tmp) - 1);
	n = sscanf(tmp, "%31[^|]|%64[^|]|%64[^|]|%64[^|]|%d|%31[^|]|%d",
		   u->username, u->salt_hex, u->pass_hash, u->rec_hash,
		   &u->attempts, lock_buf, &u->is_admin);
	if (n != 7)
		return -1;
	u->lock_until = (time_t)atoll(lock_buf);
	return 0;
}

static int format_user_line(const user_rec_t *u, char *line, size_t sz)
{
	int n = snprintf(line, sz, "%s|%s|%s|%s|%d|%lld|%d\n",
			u->username, u->salt_hex, u->pass_hash, u->rec_hash,
			u->attempts, (long long)u->lock_until, u->is_admin);
	return (n < 0 || (size_t)n >= sz) ? -1 : 0;
}

static int load_user(const char *username, user_rec_t *u, long *line_pos, long *line_len)
{
	FILE *fp;
	char line[MAX_LINE];
	long pos = 0;

	fp = fopen(USER_DB, "r");
	if (!fp)
		return -1;

	while (fgets(line, sizeof(line), fp)) {
		user_rec_t t;
		long cur_pos = pos;
		pos = ftell(fp);
		if (parse_user_line(line, &t) == 0 && strcmp(t.username, username) == 0) {
			if (u)
				*u = t;
			if (line_pos)
				*line_pos = cur_pos;
			if (line_len)
				*line_len = (pos - cur_pos);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);
	return -1;
}

static int rewrite_users(const user_rec_t *updated, const char *target_user)
{
	FILE *in, *out;
	char line[MAX_LINE];
	char formatted[MAX_LINE];
	bool replaced = false;

	in = fopen(USER_DB, "r");
	if (!in)
		return -1;
	out = fopen(USER_DB ".tmp", "w");
	if (!out) {
		fclose(in);
		return -1;
	}

	while (fgets(line, sizeof(line), in)) {
		user_rec_t t;
		if (parse_user_line(line, &t) == 0 && strcmp(t.username, target_user) == 0) {
			if (format_user_line(updated, formatted, sizeof(formatted)) < 0) {
				fclose(in);
				fclose(out);
				return -1;
			}
			fputs(formatted, out);
			replaced = true;
		} else {
			fputs(line, out);
		}
	}

	if (!replaced) {
		fclose(in);
		fclose(out);
		unlink(USER_DB ".tmp");
		return -1;
	}

	fclose(in);
	fclose(out);
	if (rename(USER_DB ".tmp", USER_DB) < 0)
		return -1;
	return 0;
}

static int append_user(const user_rec_t *u)
{
	FILE *fp;
	char line[MAX_LINE];

	if (format_user_line(u, line, sizeof(line)) < 0)
		return -1;

	fp = fopen(USER_DB, "a");
	if (!fp)
		return -1;
	fputs(line, fp);
	fclose(fp);
	return 0;
}

static int delete_user_record(const char *target_user)
{
	FILE *in, *out;
	char line[MAX_LINE];
	bool deleted = false;

	in = fopen(USER_DB, "r");
	if (!in)
		return -1;
	out = fopen(USER_DB ".tmp", "w");
	if (!out) {
		fclose(in);
		return -1;
	}

	while (fgets(line, sizeof(line), in)) {
		user_rec_t t;
		if (parse_user_line(line, &t) == 0 && strcmp(t.username, target_user) == 0) {
			deleted = true;
			continue;
		}
		fputs(line, out);
	}

	fclose(in);
	fclose(out);

	if (!deleted) {
		unlink(USER_DB ".tmp");
		return -1;
	}

	if (rename(USER_DB ".tmp", USER_DB) < 0)
		return -1;
	return 0;
}

/*
 * If users.db has no "admin" row yet, create one with known defaults printed to stderr.
 * Change passwords immediately on real deployments.
 */

static int maybe_seed_admin(void)
{
	user_rec_t admin;
	char pass_hash[HASH_HEX_LEN + 1];
	char rec_hash[HASH_HEX_LEN + 1];

	if (load_user("admin", &admin, NULL, NULL) == 0)
		return 0;

	memset(&admin, 0, sizeof(admin));
	snprintf(admin.username, sizeof(admin.username), "%s", "admin");
	if (generate_salt(admin.salt_hex) < 0)
		return -1;
	if (sha256_salted(admin.salt_hex, "Admin@1234", pass_hash) < 0)
		return -1;
	if (sha256_salted(admin.salt_hex, "admin-recovery", rec_hash) < 0)
		return -1;

	snprintf(admin.pass_hash, sizeof(admin.pass_hash), "%s", pass_hash);
	snprintf(admin.rec_hash, sizeof(admin.rec_hash), "%s", rec_hash);
	admin.attempts = 0;
	admin.lock_until = 0;
	admin.is_admin = 1;

	fprintf(stderr, "[*] Seeding default admin: admin / Admin@1234\n");
	return append_user(&admin);
}

/*
 * Block until sac_irq.c finishes TFT/LED update for the last AUTH_RESULT ioctl.
 * Each send_driver_result() pairs with this so prompts stay synchronized.
 */

static int wait_for_driver_result(int fd)
{
	struct sac_auth_result res;
	memset(&res, 0, sizeof(res));
	if (ioctl(fd, SAC_IOC_WAIT_RESULT, &res) < 0)
		return -1;
	return 0;
}

/*
 * notify_driver_waiting():
 *   Sets WAITING state via SAC_IOC_AUTH_REQUEST, then writes "auth_wait" so the
 *   driver's sac_write() hook displays AUTHENTICATING... briefly.
 * send_driver_result():
 *   Submits SAC_IOC_AUTH_RESULT for sac_irq.c to render matching TFT text and LEDs,
 *   then blocks on SAC_IOC_WAIT_RESULT until that update completes.
 */

static int notify_driver_waiting(int fd, const char *user, int ev)
{
	struct sac_auth_request req;
	char buf[32] = "auth_wait";
	ssize_t wr;

	memset(&req, 0, sizeof(req));
	req.event_type = ev;
	strncpy(req.username, user, sizeof(req.username) - 1);

	if (ioctl(fd, SAC_IOC_AUTH_REQUEST, &req) < 0)
		return -1;
	wr = write(fd, buf, strlen(buf));
	if (wr < 0)
		return -1;
	return 0;
}

static int send_driver_result(int fd, int ok, int state, const char *reason)
{
	struct sac_auth_result res;
	memset(&res, 0, sizeof(res));
	res.success = ok ? 1 : 0;
	res.state = state;
	strncpy(res.reason, reason, sizeof(res.reason) - 1);
	if (ioctl(fd, SAC_IOC_AUTH_RESULT, &res) < 0)
		return -1;
	return wait_for_driver_result(fd);
}

/* Password policy + salted comparisons against stored hashes. */

static bool strong_password(const char *p)
{
	bool up = false, lo = false, dig = false, sym = false;
	size_t i, n = strlen(p);
	if (n < 8)
		return false;
	for (i = 0; i < n; i++) {
		if (p[i] >= 'A' && p[i] <= 'Z') up = true;
		else if (p[i] >= 'a' && p[i] <= 'z') lo = true;
		else if (p[i] >= '0' && p[i] <= '9') dig = true;
		else sym = true;
	}
	return up && lo && dig && sym;
}

static int verify_password(const user_rec_t *u, const char *pass)
{
	char h[HASH_HEX_LEN + 1];
	if (sha256_salted(u->salt_hex, pass, h) < 0)
		return -1;
	return (strcmp(h, u->pass_hash) == 0) ? 0 : -1;
}

static int verify_recovery(const user_rec_t *u, const char *rec)
{
	char h[HASH_HEX_LEN + 1];
	if (sha256_salted(u->salt_hex, rec, h) < 0)
		return -1;
	return (strcmp(h, u->rec_hash) == 0) ? 0 : -1;
}

/* ---------- Interactive flows (each drives TFT via ioctl reasons) ---------- */

static int do_login(int fd)
{
	char user[SAC_MAX_USER_LEN] = {0};
	char pass[128] = {0};
	user_rec_t u;
	time_t now = time(NULL);

	if (read_line("Username: ", user, sizeof(user)) < 0) return -1;
	if (read_line("Password: ", pass, sizeof(pass)) < 0) return -1;

	if (notify_driver_waiting(fd, user, SAC_EVENT_LOGIN) < 0)
		return -1;

	if (load_user(user, &u, NULL, NULL) < 0) {
		send_driver_result(fd, 0, SAC_STATE_DENIED, "user not registered");
		audit_log("LOGIN", user, "DENIED_NO_USER");
		return -1;
	}

	if (u.lock_until > now) {
		char reason[64];
		long left = (long)(u.lock_until - now);
		if (left < 0)
			left = 0;
		snprintf(reason, sizeof(reason), "blocked %lds", left);
		send_driver_result(fd, 0, SAC_STATE_LOCKED, reason);
		audit_log("LOGIN", user, "DENIED_LOCKED");
		printf("Credentials blocked. Try again after %ld seconds.\n", left);
		return -1;
	}

	if (verify_password(&u, pass) == 0) {
		u.attempts = 0;
		u.lock_until = 0;
		rewrite_users(&u, u.username);
		send_driver_result(fd, 1, SAC_STATE_GRANTED, "ok");
		audit_log("LOGIN", user, "GRANTED");
		printf("Access granted.\n");
		return 0;
	}

	/* Lock only after three consecutive wrong password entries. */
	u.attempts++;
	if (u.attempts >= MAX_ATTEMPTS) {
		u.lock_until = now + LOCK_SECONDS;
		u.attempts = 0;
		rewrite_users(&u, u.username);
		send_driver_result(fd, 0, SAC_STATE_LOCKED, "blocked 60s");
		audit_log("LOGIN", user, "LOCKED");
		printf("Credentials blocked for %d seconds.\n", LOCK_SECONDS);
		return -1;
	}

	rewrite_users(&u, u.username);
	send_driver_result(fd, 0, SAC_STATE_DENIED, "bad password");
	audit_log("LOGIN", user, "DENIED_BAD_PASSWORD");
	printf("Access denied. Remaining retries: %d\n", MAX_ATTEMPTS - u.attempts);
	return -1;
}

/* Shared admin gate for registration / delete / print-users menus. */

static int admin_verify(int fd)
{
	char user[SAC_MAX_USER_LEN] = {0};
	char pass[128] = {0};
	user_rec_t u;

	if (read_line("Admin username: ", user, sizeof(user)) < 0) return -1;
	if (read_line("Admin password: ", pass, sizeof(pass)) < 0) return -1;

	if (notify_driver_waiting(fd, user, SAC_EVENT_REGISTER) < 0)
		return -1;

	if (load_user(user, &u, NULL, NULL) < 0 || !u.is_admin || verify_password(&u, pass) != 0) {
		send_driver_result(fd, 0, SAC_STATE_DENIED, "admin not verified");
		audit_log("ADMIN_AUTH", user, "FAILED");
		return -1;
	}

	send_driver_result(fd, 1, SAC_STATE_ADMIN_OK, "admin verified");
	audit_log("ADMIN_AUTH", user, "OK");
	return 0;
}

static int do_register(int fd)
{
	char user[SAC_MAX_USER_LEN] = {0};
	char pass[128] = {0};
	char rec[128] = {0};
	user_rec_t u;

	if (admin_verify(fd) < 0) {
		printf("Admin verification failed.\n");
		return -1;
	}
	if (send_driver_result(fd, 1, SAC_STATE_ADMIN_OK, "set username and password") < 0)
		return -1;
	printf("Admin verified. Set username and password.\n");

	if (read_line("New username: ", user, sizeof(user)) < 0) return -1;
	if (load_user(user, &u, NULL, NULL) == 0) {
		printf("User exists.\n");
		if (notify_driver_waiting(fd, user, SAC_EVENT_REGISTER) < 0)
			return -1;
		if (send_driver_result(fd, 0, SAC_STATE_DENIED, "user exists") < 0)
			return -1;
		return -1;
	}
	for (;;) {
		if (read_line("New password: ", pass, sizeof(pass)) < 0)
			return -1;
		if (strong_password(pass))
			break;
		printf("Weak password. Need upper/lower/digit/symbol and >=8 chars.\n");
		if (notify_driver_waiting(fd, user, SAC_EVENT_REGISTER) < 0)
			return -1;
		if (send_driver_result(fd, 0, SAC_STATE_DENIED, "reenter valid password") < 0)
			return -1;
	}
	if (read_line("Recovery answer/identity: ", rec, sizeof(rec)) < 0) return -1;

	memset(&u, 0, sizeof(u));
	snprintf(u.username, sizeof(u.username), "%s", user);
	if (generate_salt(u.salt_hex) < 0) return -1;
	if (sha256_salted(u.salt_hex, pass, u.pass_hash) < 0) return -1;
	if (sha256_salted(u.salt_hex, rec, u.rec_hash) < 0) return -1;
	u.attempts = 0;
	u.lock_until = 0;
	u.is_admin = 0;

	if (append_user(&u) < 0)
		return -1;

	if (notify_driver_waiting(fd, user, SAC_EVENT_REGISTER) < 0)
		return -1;
	if (send_driver_result(fd, 1, SAC_STATE_RESET_OK, "new user registered") < 0)
		return -1;

	audit_log("REGISTER", user, "SUCCESS");
	printf("User registered successfully.\n");
	return 0;
}

static int do_delete_user(int fd)
{
	char target[SAC_MAX_USER_LEN] = {0};
	user_rec_t u;

	if (admin_verify(fd) < 0) {
		printf("Admin verification failed.\n");
		return -1;
	}
	if (send_driver_result(fd, 1, SAC_STATE_ADMIN_OK, "enter username") < 0)
		return -1;
	if (read_line("Username to delete: ", target, sizeof(target)) < 0)
		return -1;

	if (strcmp(target, "admin") == 0) {
		printf("Admin account cannot be deleted.\n");
		return -1;
	}

	if (load_user(target, &u, NULL, NULL) < 0) {
		printf("User not found.\n");
		if (notify_driver_waiting(fd, target, SAC_EVENT_DELETE) < 0)
			return -1;
		if (send_driver_result(fd, 0, SAC_STATE_DENIED, "user not found") < 0)
			return -1;
		return -1;
	}

	if (delete_user_record(target) < 0) {
		audit_log("DELETE_USER", target, "FAILED");
		return -1;
	}

	if (notify_driver_waiting(fd, target, SAC_EVENT_DELETE) < 0)
		return -1;
	if (send_driver_result(fd, 1, SAC_STATE_RESET_OK, "user removed") < 0)
		return -1;

	audit_log("DELETE_USER", target, "SUCCESS");
	printf("User deleted successfully.\n");
	return 0;
}

static int do_print_users(int fd)
{
	FILE *fp;
	char line[MAX_LINE];
	user_rec_t u;
	int count = 0;

	if (admin_verify(fd) < 0) {
		printf("Admin verification failed.\n");
		return -1;
	}

	fp = fopen(USER_DB, "r");
	if (!fp) {
		printf("Cannot open users DB.\n");
		return -1;
	}

	printf("\nRegistered users:\n");
	while (fgets(line, sizeof(line), fp)) {
		if (parse_user_line(line, &u) == 0) {
			printf("- %s%s\n", u.username, u.is_admin ? " (admin)" : "");
			count++;
		}
	}
	fclose(fp);

	if (count == 0)
		printf("(no users found)\n");

	if (send_driver_result(fd, 1, SAC_STATE_RESET_OK, "user data printed") < 0)
		return -1;
	return 0;
}

/*
 * Build a serial-friendly copy: NL -> CR-NL; collapse CR+LF to one line end.
 * Caller must free(*out) when non-NULL.
 */
static int uart_build_crlf_payload(const char *buf, size_t len, char **out, size_t *out_len)
{
	char *d;
	size_t i, j;
	size_t cap;

	if (!buf || !len || !out || !out_len)
		return -1;
	*out = NULL;
	*out_len = 0;

	cap = len * 2 + 2;
	d = malloc(cap);
	if (!d)
		return -1;

	for (i = 0, j = 0; i < len; i++) {
		if (buf[i] == '\r' && i + 1 < len && buf[i + 1] == '\n') {
			i++;
			if (j + 2 >= cap)
				goto oob;
			d[j++] = '\r';
			d[j++] = '\n';
			continue;
		}
		if (buf[i] == '\n') {
			if (j + 2 >= cap)
				goto oob;
			d[j++] = '\r';
			d[j++] = '\n';
			continue;
		}
		if (buf[i] == '\r') {
			if (j + 2 >= cap)
				goto oob;
			d[j++] = '\r';
			d[j++] = '\n';
			continue;
		}
		if (j + 1 >= cap)
			goto oob;
		d[j++] = buf[i];
	}
	d[j] = '\0';
	*out = d;
	*out_len = j;
	return 0;

oob:
	free(d);
	return -1;
}

static int uart_drain_write(int ser, const char *data, size_t len)
{
	size_t off = 0;

	while (off < len) {
		ssize_t w = write(ser, data + off, len - off);

		if (w < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (w == 0)
			return -1;
		off += (size_t)w;
	}
	return tcdrain(ser);
}

/* Same as uart_drain_write but skips tcdrain (e.g. /dev/uart MMIO node). */
static int uart_write_all(int ser, const char *data, size_t len)
{
	size_t off = 0;

	while (off < len) {
		ssize_t w = write(ser, data + off, len - off);

		if (w < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (w == 0)
			return -1;
		off += (size_t)w;
	}
	return 0;
}

/*
 * Write audit text over UART0 (GPIO14 TX).
 * Prefer /dev/uart from secure_access_core (PL011 MMIO in sac_uart.c).
 * Fallback: kernel TTY nodes if MMIO driver is not loaded.
 */
static int uart_write_audit_export(const char *buf, size_t len)
{
	static const char *const paths[] = {
		"/dev/uart",
		"/dev/serial0",
		"/dev/ttyAMA0",
	};
	int ser = -1;
	struct termios tio;
	size_t p;
	char *payload = NULL;
	size_t payload_len = 0;
	const char *tx;
	size_t tx_len;

	if (!len || !buf)
		return 0;

	if (uart_build_crlf_payload(buf, len, &payload, &payload_len) != 0) {
		tx = buf;
		tx_len = len;
	} else {
		tx = payload;
		tx_len = payload_len;
	}

	for (p = 0; p < sizeof(paths) / sizeof(paths[0]); p++) {
		ser = open(paths[p], O_RDWR | O_NOCTTY | O_NONBLOCK);
		if (ser < 0)
			ser = open(paths[p], O_WRONLY | O_NOCTTY | O_NONBLOCK);
		if (ser >= 0)
			break;
	}
	if (ser < 0)
		goto fail;

	{
		int fl = fcntl(ser, F_GETFL);

		if (fl < 0)
			goto err;
		if (fcntl(ser, F_SETFL, fl & ~O_NONBLOCK) < 0)
			goto err;
	}

	if (!isatty(ser)) {
		/* /dev/uart — PL011 MMIO driver; baud set in kernel (default 9600). */
		if (uart_write_all(ser, tx, tx_len) < 0)
			goto err;
		close(ser);
		free(payload);
		return 0;
	}

	(void)ioctl(ser, TIOCEXCL);

	if (tcgetattr(ser, &tio) < 0)
		goto err;

	/* 8N1 @ 9600, no flow control; input flags cleared to avoid 7-bit strip etc. */
	tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF | IUCLC);
	tio.c_oflag &= ~OPOST;
	tio.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tio.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);
	tio.c_cflag |= CS8 | CREAD | CLOCAL;
	tio.c_cc[VMIN] = 0;
	tio.c_cc[VTIME] = 0;

	if (cfsetspeed(&tio, B9600) != 0)
		goto err;
	if (tcsetattr(ser, TCSAFLUSH, &tio) < 0)
		goto err;

	if (uart_drain_write(ser, tx, tx_len) < 0)
		goto err;

	(void)ioctl(ser, TIOCNXCL);
	close(ser);
	free(payload);
	return 0;

err:
	(void)ioctl(ser, TIOCNXCL);
	{
		int saved = errno;

		close(ser);
		errno = saved;
	}
fail:
	free(payload);
	return -1;
}

/*
 * Parse audit line time for the last-24h filter.
 * Legacy: leading "epoch|..." (CLOCK_REALTIME seconds).
 * New: "YYYY-MM-DD HH:MM:SS|event|..." (local wall, no epoch).
 * Legacy UTC: "YYYY-MM-DD HH:MM:SS UTC|...".
 */
static int audit_line_time(const char *line, time_t *tp)
{
	struct tm tm;
	const char *rest;
	char *endp;
	long long ep;

	ep = strtoll(line, &endp, 10);
	if (endp != line && *endp == '|' && ep >= 946684800LL && ep <= 4102444800LL) {
		*tp = (time_t)ep;
		return 0;
	}

	memset(&tm, 0, sizeof(tm));
	rest = strptime(line, "%Y-%m-%d %H:%M:%S|", &tm);
	if (rest) {
		tm.tm_isdst = -1;
		*tp = mktime(&tm);
		return (*tp == (time_t)-1) ? -1 : 0;
	}

	memset(&tm, 0, sizeof(tm));
	rest = strptime(line, "%Y-%m-%d %H:%M:%S UTC|", &tm);
	if (rest) {
		*tp = timegm(&tm);
		return (*tp == (time_t)-1) ? -1 : 0;
	}

	return -1;
}

#define AUDIT_EXPORT_MAX ((size_t)2 * 1024 * 1024)

/* Admin-gated: entries from the last 24 hours over UART; *out is malloc'd (or NULL); caller frees. */
static int fill_audit_last_24h(char **out, size_t *olen)
{
	FILE *fp;
	char line[MAX_LINE];
	time_t now, cutoff;
	char *buf = NULL;
	size_t cap = 0;
	size_t len = 0;
	bool truncated = false;

	*out = NULL;
	*olen = 0;

	fp = fopen(AUDIT_DB, "r");
	if (!fp)
		return -1;

	now = time(NULL);
	cutoff = now - (time_t)86400;

	while (fgets(line, sizeof(line), fp)) {
		time_t t;
		size_t l = strlen(line);

		if (audit_line_time(line, &t) < 0)
			continue;
		if (t < cutoff)
			continue;
		if (len + l + 1 > AUDIT_EXPORT_MAX) {
			truncated = true;
			break;
		}
		if (len + l + 1 > cap) {
			size_t ncap = cap == 0 ? (size_t)16384 : cap * 2;

			while (len + l + 1 > ncap)
				ncap *= 2;
			if (ncap > AUDIT_EXPORT_MAX)
				ncap = AUDIT_EXPORT_MAX;
			{
				char *nb = realloc(buf, ncap);

				if (!nb) {
					fclose(fp);
					free(buf);
					return -1;
				}
				buf = nb;
				cap = ncap;
			}
		}
		memcpy(buf + len, line, l);
		len += l;
		buf[len] = '\0';
	}
	fclose(fp);

	if (truncated && buf) {
		static const char suf[] = "\n... [audit export truncated to size limit]\n";
		size_t sl = sizeof(suf) - 1;

		if (len + sl + 1 <= AUDIT_EXPORT_MAX) {
			char *nb = realloc(buf, len + sl + 1);

			if (nb) {
				buf = nb;
				memcpy(buf + len, suf, sl);
				len += sl;
				buf[len] = '\0';
			}
		}
	}

	*out = buf;
	*olen = len;
	return 0;
}

static int do_print_audit_logs(int fd)
{
	char *export_buf = NULL;
	size_t n = 0;

	if (admin_verify(fd) < 0) {
		printf("Admin verification failed.\n");
		return -1;
	}

	if (fill_audit_last_24h(&export_buf, &n) < 0) {
		printf("Cannot read audit log.\n");
		return -1;
	}

	{
		const char *empty_msg = "(no audit entries in last 24 hours)\r\n";

		if (n == 0) {
			if (uart_write_audit_export(empty_msg, strlen(empty_msg)) < 0)
				fprintf(stderr, "UART export failed: %s\n", strerror(errno));
		} else if (uart_write_audit_export(export_buf, n) < 0) {
			fprintf(stderr, "UART export failed: %s\n", strerror(errno));
		}
	}

	free(export_buf);

	if (send_driver_result(fd, 1, SAC_STATE_RESET_OK, "audit logs printed") < 0)
		return -1;
	return 0;
}

static int do_forgot(int fd)
{
	char user[SAC_MAX_USER_LEN] = {0};
	char rec[128] = {0};
	char npass[128] = {0};
	user_rec_t u;

	if (read_line("Username: ", user, sizeof(user)) < 0) return -1;
	if (read_line("Recovery answer/identity: ", rec, sizeof(rec)) < 0) return -1;

	if (notify_driver_waiting(fd, user, SAC_EVENT_FORGOT) < 0)
		return -1;

	if (load_user(user, &u, NULL, NULL) < 0) {
		send_driver_result(fd, 0, SAC_STATE_DENIED, "user not found");
		audit_log("FORGOT", user, "FAILED");
		return -1;
	}
	if (verify_recovery(&u, rec) != 0) {
		send_driver_result(fd, 0, SAC_STATE_DENIED, "recovery failed");
		audit_log("FORGOT", user, "FAILED");
		return -1;
	}

	if (send_driver_result(fd, 1, SAC_STATE_ADMIN_OK, "enter new password") < 0)
		return -1;
	for (;;) {
		if (read_line("New password: ", npass, sizeof(npass)) < 0)
			return -1;
		if (strong_password(npass))
			break;
		printf("Weak password. Need upper/lower/digit/symbol and >=8 chars.\n");
		if (notify_driver_waiting(fd, user, SAC_EVENT_FORGOT) < 0)
			return -1;
		if (send_driver_result(fd, 0, SAC_STATE_DENIED, "enter valid password") < 0)
			return -1;
	}

	if (sha256_salted(u.salt_hex, npass, u.pass_hash) < 0)
		return -1;
	u.attempts = 0;
	u.lock_until = 0;
	if (rewrite_users(&u, u.username) < 0)
		return -1;

	send_driver_result(fd, 1, SAC_STATE_RESET_OK, "reset ok");
	audit_log("FORGOT", user, "RESET_SUCCESS");
	printf("Password reset success.\n");
	return 0;
}

/* Open driver node, seed storage, dispatch menu until EOF or exit choice. */

int main(void)
{
	int fd;
	char c[8];

	tzset();

	if (ensure_storage() < 0 || maybe_seed_admin() < 0) {
		fprintf(stderr, "Storage init failed: %s\n", strerror(errno));
		return 1;
	}

	fd = open(DEV_NODE, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		fprintf(stderr, "Open %s failed: %s\n", DEV_NODE, strerror(errno));
		return 1;
	}

	while (1) {
		printf("\n=== Secure Access Menu ===\n");
		printf("1. User Login\n2. User Registration\n3. Forgot Password\n4. Delete User\n5. Print Users\n6. Print Audit Logs (UART, last 24h)\n7. Exit\nChoice: ");
		if (!fgets(c, sizeof(c), stdin))
			break;

		switch (atoi(c)) {
		case 1:
			(void)do_login(fd);
			break;
		case 2:
			(void)do_register(fd);
			break;
		case 3:
			(void)do_forgot(fd);
			break;
		case 4:
			(void)do_delete_user(fd);
			break;
		case 5:
			(void)do_print_users(fd);
			break;
		case 6:
			(void)do_print_audit_logs(fd);
			break;
		case 7:
			(void)ioctl(fd, SAC_IOC_CLEAR_TFT);
			close(fd);
			return 0;
		default:
			printf("Invalid choice.\n");
			break;
		}
	}

	close(fd);
	return 0;
}
