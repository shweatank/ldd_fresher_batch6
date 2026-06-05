
obj-m +=kernel_proc.o

all:
	make -C/lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -c/lib?modules$(shell uname -r)/build M=$(PWD) clean

