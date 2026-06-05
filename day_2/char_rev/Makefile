
obj-m +=basic2.o


KDIR:=~/Desktop/mirafra_train/training/day_2/char_rev
PWD :=$(shell pwd)
all:
	make -C $(KDIR) M=$(PWD) ARCH=arm64
	CROSS_COMPILE = aarch64-linux-gnu-modules

clean:
	make -C $(KDIR) M=$(PWD) ARCH=arm64
	CROSS_COMPILE = aarch64-linux-gnu-clean

