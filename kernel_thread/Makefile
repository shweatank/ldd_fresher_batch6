
obj-m +=kernel_thread.o

all:
	make -C/lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -c/lib?modules$(shell uname -r)/build M=$(PWD) clean

