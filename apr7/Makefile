obj-m += ldd1.o

KDIR:=/home/mirafra/linux #path  to kernel folder

all:
	make -C  $(KDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules

clean:
	make -C $(KDIR) M=$(PWD) clean
