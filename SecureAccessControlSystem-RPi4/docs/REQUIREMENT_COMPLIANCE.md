# Requirement Compliance Matrix

## Core System
- User login: implemented in `user/secure_access_app.c`.
- User registration with admin authentication: implemented.
- Forgot password recovery: implemented.
- Password hashing + salt: SHA-256 salted via OpenSSL EVP.
- SD card storage (`users.db`, `audit.log`): implemented.

## Driver + Kernel Concepts
- Character driver (`open/read/write/ioctl/release`): implemented in `driver/sac_core_main.c`.
- Interrupt top-half: `sac_irq_handler()`.
- Bottom half (tasklet): `sac_tasklet_fn()` (minimal scheduling only).
- Deferred heavy work: moved to workqueue (`sac_auth_work_fn()`) to allow sleeps safely.
- Wait queues:
  - wake: `wake_up_interruptible()` in deferred worker.
  - block: `wait_event_interruptible()` via `SAC_IOC_WAIT_RESULT`.
- Synchronization:
  - mutex (`io_lock`), spinlock (`state_lock`), atomic vars, semaphore.

## SPI TFT + GPIO
- ILI9225 SPI init and command/data path: `driver/sac_tft.c`.
- Text rendering: draw pixel/char/text with wrap and scale 1x/2x.
- Runtime style ioctl: `SAC_IOC_SET_TFT_STYLE`.
- LEDs via gpiod APIs: green/red/wait behavior implemented.

## Device Tree + Platform + IOREMAP
- OF matching + platform driver: implemented (`compatible = "secure,sac-ctrl"`).
- DTS overlay for SPI, GPIO, IRQ, reg resource: implemented in `dt/secure-access-overlay.dts`.
- UART audit export: `ioremap` PL011 + `readl`/`writel` in `driver/sac_uart.c` (`/dev/uart`, 9600 baud).

## Bonus/Interview Features
- Poll/select support: implemented in `sac_poll()`.
- Sysfs runtime stats: `/sys/devices/platform/.../stats`.
- Kernel monitor thread: `sac_monitor_thread_fn()`.
- Audit logging + account lock timeout + password strength check: implemented.

## Note
- Credential verification policy is intentionally in userspace app (clean separation and easier storage/security maintenance), while kernel driver provides secure signaling, synchronization, IRQ/deferred handling, and hardware control.
