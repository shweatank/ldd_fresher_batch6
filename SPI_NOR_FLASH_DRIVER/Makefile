KDIR := /home/mirafra/linux-6.1.22

ARCH := arm64
CROSS_COMPILE := aarch64-linux-gnu-

obj-m := w25q128.o

APP := test
APP_SRC := flash_app.c

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
