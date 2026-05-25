obj-m += ems_driver.o

ems_driver-objs := \
	ems_main.o \
	dht11.o \
	ssd1306_oled_i2c.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

