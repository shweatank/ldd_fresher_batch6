CC := gcc
CFLAGS := -Wall -Wextra -O2 -I../include -pthread
LDFLAGS := -pthread -lrt

TARGETS := daemon logger dashboard alert_manager

all: $(TARGETS)

daemon: daemon.c common.h ../include/sensor_ioctl.h
	$(CC) $(CFLAGS) -o $@ daemon.c $(LDFLAGS)

logger: logger.c common.h
	$(CC) $(CFLAGS) -o $@ logger.c $(LDFLAGS)

dashboard: dashboard.c common.h
	$(CC) $(CFLAGS) -o $@ dashboard.c $(LDFLAGS)

alert_manager: alert_manager.c common.h ../include/sensor_ioctl.h
	$(CC) $(CFLAGS) -o $@ alert_manager.c $(LDFLAGS)

clean:
	rm -f $(TARGETS) *.o *.log
