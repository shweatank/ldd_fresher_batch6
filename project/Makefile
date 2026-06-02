obj-m += rtc_project.o

rtc_project-objs := \
main.o \
gpio.o \
uart.o \
lcd.o \
rtc.o \
spi_adc.o

KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

