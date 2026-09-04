.PHONY: all driver app clean

KDIR := /home/mirafra/imx8mn-spi/linux-6.1.22

ARCH := arm64
CROSS_COMPILE := aarch64-linux-gnu-

obj-m := w25q128.o

APP_SRC := test.c
APP := test

all: driver app

driver:
	$(MAKE) -C $(KDIR) \
		ARCH=$(ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		M=$(PWD) \
		modules

app:
	$(CROSS_COMPILE)gcc -Wall -Wextra \
		$(APP_SRC) -o $(APP)

clean:
	$(MAKE) -C $(KDIR) \
		ARCH=$(ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		M=$(PWD) \
		clean
	rm -f $(APP)
