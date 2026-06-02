# Secure Access Control System using Linux Device Drivers, ILI9225, IRQ, Tasklets, Wait Queues, DTS, IOREMAP, SD Card

Production-style embedded Linux project for **Raspberry Pi 4 Model B**.

Implements:
- User login, registration (admin-gated), forgot password
- Salted SHA-256 credential handling
- SD card-backed credential + audit storage
- Linux character device driver (`/dev/secure_access0`)
- Interrupt top-half + tasklet bottom-half
- Wait queue synchronization + `poll/select`
- GPIO LED signaling (`green`, `red`)
- ILI9225 SPI TFT status updates
- Device Tree overlay for SPI/GPIO/IRQ
- PL011 UART0 audit export via `ioremap` (`/dev/uart` in `sac_uart.c`, 9600 baud)

## Folder Structure

```text
SecureAccessControlSystem-RPi4/
├── driver/
│   ├── include/
│   │   ├── secure_access_drv.h
│   │   └── secure_access_ioctl.h
│   ├── sac_core_main.c
│   ├── sac_irq.c
│   ├── sac_uart.c
│   ├── sac_tft.c
│   └── Makefile
├── user/
│   ├── secure_access_app.c
│   └── Makefile
├── dt/
│   └── secure-access-overlay.dts
├── docs/
│   └── architecture.md
└── README.md
```

## Device Tree Concepts Used

- `compatible`, `reg`, `status = "okay"`
- SPI node (`ili9225@0`) with `spi-max-frequency`
- GPIO properties (`green-gpios`, `red-gpios`, `dc-gpios`, `reset-gpios`, `backlight-gpios`)
- Interrupt mapping (`interrupt-parent`, `interrupts`)

## Build

From the repository root (adjust the path to your clone):

```bash
cd ~/raspberry/SecureAccessControlSystem-RPi4
make -C driver
make -C user
```

## Overlay compile

Install the compiler once if needed: `sudo apt install device-tree-compiler`

```bash
mkdir -p dt/out
dtc -@ -I dts -O dtb -o dt/out/secure-access-overlay.dtbo dt/secure-access-overlay.dts
```

Copy generated overlay (Bookworm uses `/boot/firmware`; older images use `/boot`):

```bash
sudo cp dt/out/secure-access-overlay.dtbo /boot/firmware/overlays/
# or: sudo cp dt/out/secure-access-overlay.dtbo /boot/overlays/
```

Add to `/boot/config.txt`:

```text
dtoverlay=secure-access-overlay
```

Enable SPI in `/boot/config.txt` too:

```text
dtparam=spi=on
```

Reboot after configuration.

## Module load / unload

From the repository root:

```bash
sudo insmod driver/secure_access_core.ko
lsmod | grep secure_access_core
ls -l /dev/secure_access0
```

Unload:

```bash
sudo rmmod secure_access_core
```

## User App Run

```bash
cd user
make
sudo ./secure_access_app
```

> Storage path: `/mnt/sdcard/secure_access`
>
> Default seeded admin on first run: `admin / Admin@1234`

## Authentication Sequence

1. User app sends `SAC_IOC_AUTH_REQUEST` (state=WAITING)
2. Driver displays `AUTHENTICATING...`
3. User app verifies credentials against salted SHA-256 DB
4. User app sends `SAC_IOC_AUTH_RESULT`
5. Driver triggers IRQ/software event
6. ISR schedules tasklet
7. Tasklet updates TFT + LEDs and wakes waiting readers (`wake_up_interruptible`)

## Driver APIs Demonstrated

- Character device: `open/read/write/ioctl/release`
- Wait queues: `wait_event_interruptible` semantics via poll + wake
- Interrupts: `request_irq` (through `devm_request_irq`), `free_irq` managed cleanup
- Bottom half: tasklet scheduling
- Sync: mutex, spinlock, atomic, semaphore
- SPI transfers: `spi_sync`
- GPIO descriptor API: `devm_gpiod_get_optional`, `gpiod_set_value_cansleep`
- MMIO: `platform_get_resource`, `devm_ioremap_resource`, `readl`, `writel`
- Runtime visibility: sysfs stats + kernel monitor thread
- TFT text renderer: `draw_pixel`, `draw_char`, `draw_text` with built-in 5x7 glyphs
- TFT enhancements: wrapped multi-line text, scale `1x/2x`, runtime style control via `SAC_IOC_SET_TFT_STYLE`

### Runtime stats

Read live counters:

```bash
cat /sys/devices/platform/secure-access-ctrl@0/stats
```

Example fields include total auth requests, success/failure count, lock count, IRQ count, tasklet runs, and wakeups.

## Security Notes

- Plain-text passwords are never stored.
- Each user has unique random 32-byte salt (hex encoded).
- Stored fields: username, salt, password hash, recovery hash, attempts, lock timestamp, admin flag.
- File permissions set to `0600` for credential and audit files.
- Includes input validation, bounded buffers, and safe string handling.

## Linux Verification Commands

```bash
dmesg | tail -n 100
lsmod | grep secure_access
sudo insmod driver/secure_access_core.ko
sudo rmmod secure_access_core
sudo modprobe spi_bcm2835
ls /dev/secure_access0
ls /sys/bus/spi/devices/
ls /proc/device-tree/
```

## Debug Checklist

- No `/dev/secure_access0`: check probe logs in `dmesg` and DT overlay loaded.
- SPI missing: verify `dtparam=spi=on` and `ls /sys/bus/spi/devices`.
- TFT not updating: verify DC/RESET/BACKLIGHT GPIO mapping and SPI mode/frequency.
- IRQ not firing: validate GPIO interrupt pin and edge type in DTS.
- SD storage error: ensure `/mnt/sdcard` mounted and writable.

## Why `devm_ioremap_resource()` over raw `ioremap()`

- Automatic cleanup on probe failure/remove.
- Validates resource region ownership.
- Less error-prone resource lifetime handling.
- Reduces memory/resource leaks and double-unmap mistakes.

Common ioremap mistakes:
- Mapping wrong physical range/size.
- Access without barriers (`readl/writel` should be used).
- Forgetting unmap path with manual `ioremap`.
- Mixing cached vs non-cached access assumptions.

