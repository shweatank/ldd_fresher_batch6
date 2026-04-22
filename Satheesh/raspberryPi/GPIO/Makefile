obj-m += read_gpio.o

KDIR := /home/mirafra/ldd_fresher_batch6/Satheesh/raspberryPi/linux
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules

clean:
	make -C $(KDIR) M=$(PWD) clean
