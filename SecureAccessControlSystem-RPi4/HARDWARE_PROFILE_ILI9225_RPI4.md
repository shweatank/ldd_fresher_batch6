# Hardware-Specific Profile: ILI9225 + Raspberry Pi 4 (No-Guess Fallback)

This profile is for the common 176x220 ILI9225 SPI modules. Use this when exact board revision is unknown.

## 1) Identify your module type by pin labels

### Type A (most common)
Header labels like:
- `VCC GND CS RST RS SDA SCK LED`
- where `RS` = `DC`, `SDA` = `MOSI`

### Type B
Header labels like:
- `VCC GND CS RESET DC SDI SCK BL`

### Type C (with extras)
- Display pins plus `SDO/MISO`, `T_CS`, `T_IRQ`, `T_CLK`, `T_DIN`, `T_DO`, `SD_CS`

For this project, only display pins are required.

---

## 2) Exact Pi mapping for current project DTS/code

Use this mapping exactly (matches `dt/secure-access-overlay.dts`):

- `VCC` -> `Pi 3.3V` (Pin 1)
- `GND` -> `Pi GND` (Pin 6)
- `SCK/SCL/CLK` -> `GPIO11` (Pin 23)
- `SDA/SDI/MOSI/DIN` -> `GPIO10` (Pin 19)
- `CS` -> `GPIO8` (Pin 24)
- `RS/DC` -> `GPIO24` (Pin 18)
- `RST/RESET` -> `GPIO25` (Pin 22)
- `LED/BL` -> `GPIO23` (Pin 16)

### LEDs and IRQ (project-specific)
- Green LED -> GPIO17 (Pin 11)
- Red LED -> GPIO27 (Pin 13)
- Wait LED -> GPIO22 (Pin 15)
- IRQ input -> GPIO5 (Pin 29)

---

## 3) Voltage and safety rules (critical)

- Prefer modules that are **3.3V logic compatible**.
- If module is 5V-only logic, add proper level shifting for control/SPI lines.
- Never leave grounds floating.
- Use short jumper wires for SPI stability.

---

## 4) If your module has `MISO/SDO`

- Keep it unconnected for this project (display writes only).
- Do not change driver for MISO unless you add readback support.

---

## 5) If your board also has touch or SD slot

- Leave touch/SD pins disconnected unless you intentionally add support.
- Only connect display SPI + control pins listed above.

---

## 6) Boot + run sequence (hardware-specific quick run)

```bash
cd ~/Documents/SecureAccessControlSystem-RPi4
make -C driver
make -C user
mkdir -p dt/out
dtc -@ -I dts -O dtb -o dt/out/secure-access-overlay.dtbo dt/secure-access-overlay.dts
sudo cp dt/out/secure-access-overlay.dtbo /boot/firmware/overlays/
# older images: sudo cp dt/out/secure-access-overlay.dtbo /boot/overlays/
```

Ensure `/boot/config.txt` contains:

```text
dtparam=spi=on
dtoverlay=secure-access-overlay
```

Reboot and run:

```bash
cd ~/Documents/SecureAccessControlSystem-RPi4
sudo insmod driver/secure_access_core.ko
cd user
sudo ./secure_access_app
```

---

## 7) First-power validation logic

1. Backlight ON? If no, check `LED/BL -> GPIO23` and power.
2. `/dev/secure_access0` exists? If no, check overlay + dmesg.
3. LEDs blink on success/failure? If no, verify GPIO17/27 wiring.
4. TFT text appears? If no, re-check DC/RST/CS pin mapping and SPI pins.

---

## 8) Exact fixes by symptom

### Symptom: white/blank screen
- Swap only DC and RST once (some modules mislabel RS/RST).
- Lower SPI speed in DTS from `10000000` to `4000000`, rebuild overlay.

### Symptom: garbled text/colors
- Confirm SPI mode 0 wiring and solid GND.
- Keep wires shorter.

### Symptom: module loads but no TFT updates
- Check if module is actually ST77xx/ILI9341 clone mislabeled as ILI9225.
- If mislabeled, initialization sequence must be changed.

---

