# Kernel module Makefile for out-of-tree modules
#
# secure_access_core.ko — platform driver + misc /dev/uart PL011 MMIO (sac_uart.c)
#
# Build from driver/: make
# Or from repo root: make driver   /   make -C driver

obj-m += secure_access_core.o

secure_access_core-objs := sac_core_main.o sac_tft.o sac_irq.o sac_uart.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
