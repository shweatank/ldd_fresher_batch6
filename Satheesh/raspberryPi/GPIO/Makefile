<<<<<<< HEAD
obj-m += read_blink.o

KDIR := /home/mirafra/linux
PWD  := $(shell pwd)

ARCH := arm64
CROSS_COMPILE := aarch64-linux-gnu-

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
=======
obj-m+=ioctl_led.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
>>>>>>> 6d64dae (ioctl_GPIO)

