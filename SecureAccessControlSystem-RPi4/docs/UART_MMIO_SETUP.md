# UART MMIO setup (GPIO14 TX / `/dev/uart`)

The audit export uses **PL011 UART0** through **`sac_uart.c`** inside `secure_access_core.ko`, which creates **`/dev/uart`** (same idea as your `bcm2711_gpio_uart` sample).

## Wiring

- Pi **GPIO14** (TX) → USB–TTL **RX**
- Pi **GPIO15** (RX) → USB–TTL **TX** (optional; audit export is mostly TX)
- **GND** common

## Baud rate

- Default: **9600** (fixed in `sac_uart.c`; match minicom).

## Avoid two drivers on one UART

Do **not** use kernel **`ttyAMA0` / `serial0` console** and this MMIO driver at the same time.

On the Pi:

1. `sudo raspi-config` → Interface Options → Serial → login shell **No**, hardware **Yes**
2. Or disable: `sudo systemctl disable --now serial-getty@ttyAMA0.service`
3. Load only: `sudo insmod secure_access_core.ko`
4. Check: `ls -l /dev/uart /dev/secure_access0`

## Test

```bash
# PC
minicom -D /dev/ttyUSB0 -b 9600

# Pi — after insmod + run app, menu 6 Print Audit Logs (admin)
```

## Userspace path

`secure_access_app` opens **`/dev/uart`** first, then falls back to `/dev/serial0` / `ttyAMA0` if needed.
