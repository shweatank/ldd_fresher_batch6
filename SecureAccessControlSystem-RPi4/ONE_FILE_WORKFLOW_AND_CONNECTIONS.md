# Secure Access Control System - Exact Workflow and Connections (Raspberry Pi 4)

This is the single authoritative runbook for wiring + bring-up.

Important reality: no embedded setup can be guaranteed "perfect" without your exact TFT breakout variant and power/wiring quality. This guide is written to be as exact and deterministic as possible for the current codebase.

---

## 0) Preconditions (must match)

- Board: Raspberry Pi 4 Model B
- OS: Raspberry Pi OS / Debian-based Linux with kernel headers installed
- Project path: `~/Documents/SecureAccessControlSystem-RPi4`
- SPI enabled through boot config
- SD card mounted at: `/mnt/sdcard`
- You can run `sudo`

Install build dependencies once:

```bash
sudo apt update
sudo apt install -y build-essential raspberrypi-kernel-headers device-tree-compiler libssl-dev
```

Ensure SD mount exists and is writable:

```bash
sudo mkdir -p /mnt/sdcard
mount | rg "/mnt/sdcard" || echo "Mount SD card to /mnt/sdcard before app run"
```

---

## 1) Exact Wiring Table (physical pin numbers)

### 1.1 ILI9225 SPI TFT

| TFT Pin | Raspberry Pi Pin | BCM GPIO | Notes |
|---|---:|---:|---|
| VCC | 1 | 3.3V | Do NOT use 5V unless your module explicitly supports it |
| GND | 6 | GND | Common ground mandatory |
| SCK/CLK | 23 | GPIO11 | SPI0_SCLK |
| SDI/MOSI | 19 | GPIO10 | SPI0_MOSI |
| CS | 24 | GPIO8 | SPI0_CE0 |
| DC/RS | 18 | GPIO24 | TFT command/data select |
| RESET/RST | 22 | GPIO25 | TFT reset line |
| LED/BL | 16 | GPIO23 | Backlight control from driver |

### 1.2 LEDs (with 220 ohm resistor each)

| LED | GPIO | Physical Pin | Wiring |
|---|---:|---:|---|
| Green | GPIO17 | 11 | Anode -> resistor -> GPIO17, cathode -> GND |
| Red | GPIO27 | 13 | Anode -> resistor -> GPIO27, cathode -> GND |
| Wait | GPIO22 | 15 | Anode -> resistor -> GPIO22, cathode -> GND |

### 1.3 Interrupt input (optional hardware trigger)

| Signal | GPIO | Physical Pin | Notes |
|---|---:|---:|---|
| IRQ input | GPIO5 | 29 | DTS-configured interrupt pin |

Optional button test: connect button between GPIO5 and GND and use pull-up configuration externally or through pin config.

---

## 2) DTS Mapping used by this project

File: `dt/secure-access-overlay.dts`

- SPI device node: `ili9225@0`
  - `compatible = "ilitek,ili9225"`
  - `reg = <0>`
  - `spi-max-frequency = <10000000>`
- Platform node: `secure-access-ctrl@0`
  - `compatible = "secure,sac-ctrl"`
  - `green-gpios = <&gpio 17 0>`
  - `red-gpios = <&gpio 27 0>`
  - `wait-gpios = <&gpio 22 0>`
  - `dc-gpios = <&gpio 24 0>`
  - `reset-gpios = <&gpio 25 0>`
  - `backlight-gpios = <&gpio 23 0>`
  - `interrupt-parent = <&gpio>`
  - `interrupts = <5 0x2>`
  - `reg = <0x0 0xfe200000 0x0 0x1000>` for MMIO demonstration

---

## 3) Exact one-time bring-up workflow

Run exactly in order.

### Step 1 - Build project

```bash
cd ~/Documents/SecureAccessControlSystem-RPi4
make -C driver
make -C user
```

Expected:
- `driver/secure_access_core.ko` exists
- `user/secure_access_app` exists

Check:

```bash
ls -l driver/secure_access_core.ko user/secure_access_app
```

### Step 2 - Compile DTS overlay

```bash
mkdir -p dt/out
dtc -@ -I dts -O dtb -o dt/out/secure-access-overlay.dtbo dt/secure-access-overlay.dts
ls -l dt/out/secure-access-overlay.dtbo
```

### Step 3 - Install overlay and configure boot

```bash
sudo cp dt/out/secure-access-overlay.dtbo /boot/firmware/overlays/
# older images: sudo cp dt/out/secure-access-overlay.dtbo /boot/overlays/
```

Edit `/boot/config.txt` and ensure these lines exist once:

```text
dtparam=spi=on
dtoverlay=secure-access-overlay
```

Then reboot:

```bash
sudo reboot
```

### Step 4 - Post-reboot validation

```bash
ls /sys/bus/spi/devices/
```

You should see SPI devices (e.g. `spi0.0`).

```bash
ls /proc/device-tree | rg -i "soc|__symbols__|chosen"
```

### Step 5 - Load module

```bash
cd ~/Documents/SecureAccessControlSystem-RPi4
sudo insmod driver/secure_access_core.ko
lsmod | grep secure_access_core || true
ls -l /dev/secure_access0 || true
dmesg | tail -n 40
```

Mandatory checks:

```bash
lsmod | rg secure_access_core
ls -l /dev/secure_access0
dmesg | tail -n 120
```

Expected dmesg clues:
- driver probed successfully
- optional MMIO demo log
- optional TFT init logs

### Step 6 - Run application

```bash
cd ~/Documents/SecureAccessControlSystem-RPi4/user
sudo ./secure_access_app
```

---

## 4) Exact runtime workflow (software path)

1. App sends `SAC_IOC_AUTH_REQUEST`.
2. Driver sets waiting state, wait LED ON, TFT `AUTHENTICATING...`.
3. App validates credentials from `/mnt/sdcard/secure_access/users.db` using salted SHA-256.
4. App sends `SAC_IOC_AUTH_RESULT`.
5. Driver triggers IRQ top-half (`sac_irq_handler`).
6. Tasklet runs minimal logic and schedules workqueue.
7. Workqueue performs LED blink + TFT final status, then `wake_up_interruptible()`.
8. App blocks using `SAC_IOC_WAIT_RESULT` (internally `wait_event_interruptible()`), then continues.

---

## 5) Functional test script (manual)

### First run
- Default admin auto-created if missing:
  - user: `admin`
  - pass: `Admin@1234`

### Test A - Registration (admin-gated)
1. Choose menu `2`
2. Enter admin credentials
3. Enter new username
4. Enter strong password (upper+lower+digit+symbol, >=8)
5. Enter recovery text
6. Confirm success + audit log entry

### Test B - Login success
1. Choose menu `1`
2. Enter valid user/pass
3. Expect:
   - TFT: ACCESS GRANTED
   - Green LED blinks
   - auth success stats increment

### Test C - Login failure and lock
1. Try wrong password repeatedly
2. After configured attempts, expect:
   - TFT: ACCOUNT LOCKED
   - Red LED blinks
   - lock timeout enforced

### Test D - Forgot password
1. Choose menu `3`
2. Enter username + correct recovery
3. Set new strong password
4. Expect PASSWORD RESET status and successful login after reset

### Test E - TFT style
1. Choose menu `4`
2. Select style preset
3. Trigger status message (login/register/forgot) and verify color/scale change

---

## 6) Exact verification commands

```bash
# Module/device
lsmod | rg secure_access_core
ls -l /dev/secure_access0

# SPI and DT
ls /sys/bus/spi/devices/
ls /proc/device-tree/

# Driver logs
sudo dmesg | tail -n 200

# Runtime stats (path can vary by kernel naming)
ls /sys/devices/platform | rg secure
cat /sys/devices/platform/secure-access-ctrl@0/stats 2>/dev/null || true
```

Stats fields include request/success/failure/locked/irq/tasklet/wakeups/open/state.

---

## 7) Storage files and exact format

Location:
- `/mnt/sdcard/secure_access/users.db`
- `/mnt/sdcard/secure_access/audit.log`

`users.db` record format:

```text
username|salt_hex|pass_hash_hex|recovery_hash_hex|attempts|lock_until_epoch|is_admin
```

Security guarantees in current implementation:
- plaintext passwords are never stored
- per-user random salt
- salted SHA-256 for password and recovery data
- file mode set to `0600`

---

## 8) Failure recovery (exact actions)

### Module fails to load

```bash
sudo dmesg | tail -n 200
```

Common causes:
- overlay not loaded
- SPI not enabled
- GPIO pin conflict

### `/dev/secure_access0` missing

- Probe likely failed; inspect dmesg for `secure access controller` messages.

### TFT blank but app works

- Verify DC/RESET/BL pins and module voltage (3.3V logic).
- Try reducing SPI frequency in DTS and rebuild overlay.

### SD files not created

- Ensure SD is mounted writable at `/mnt/sdcard`.

### Start from clean module state

```bash
cd ~/Documents/SecureAccessControlSystem-RPi4
sudo rmmod secure_access_core || true
sudo insmod driver/secure_access_core.ko
```

---

## 9) “Run perfectly” checklist

Before demo/interview, all must pass:
- [ ] `driver/secure_access_core.ko` builds
- [ ] `user/secure_access_app` builds
- [ ] overlay compiled and copied
- [ ] `/boot/config.txt` has `dtparam=spi=on` and `dtoverlay=secure-access-overlay`
- [ ] reboot completed
- [ ] `/dev/secure_access0` exists
- [ ] app launches and admin default works
- [ ] login success/failure/lock/forgot all tested once
- [ ] TFT and LEDs visibly respond

