# Main project Makefile — builds kernel drivers and userspace app.
#
# Outputs (from driver/):
#   secure_access_core.ko  — platform /dev/secure_access0 + misc /dev/uart (MMIO)
#
# Usage:
#   make              — build driver + user
#   make driver       — build only kernel module (secure_access_core.ko)
#   make user         — build only secure_access_app
#   make clean        — clean driver and user build artifacts

.PHONY: all driver user clean

all: driver user

driver:
	$(MAKE) -C driver

user:
	$(MAKE) -C user

clean:
	$(MAKE) -C driver clean
	$(MAKE) -C user clean
