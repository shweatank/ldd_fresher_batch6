obj-m += read_blink.o

KDIR := /home/mirafra/linux
PWD  := $(shell pwd)

ARCH := arm64
CROSS_COMPILE := aarch64-linux-gnu-

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

