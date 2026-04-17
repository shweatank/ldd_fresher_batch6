obj-m +=char_kmalloc_driver_cal.o

all:
	make -C/lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C/lib/modules/$(shell uname -r)/build M=$(PWD) clean
