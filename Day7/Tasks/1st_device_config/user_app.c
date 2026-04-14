#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include "config_ioctl.h"

int main() {
    int fd = open("/dev/config_dev", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    struct device_config my_cfg = {2, 500000, "HighSpeed_Node"};
    
    // 1. SET
    printf("Setting new config...\n");
    ioctl(fd, SET_CONFIG, &my_cfg);

    // 2. RESET
    printf("Resetting to default...\n");
    ioctl(fd, RESET_CONFIG);

    // 3. GET
    struct device_config fetched;
    ioctl(fd, GET_CONFIG, &fetched);
    printf("Current Config -> Mode: %d, Speed: %d, Name: %s\n", 
            fetched.mode, fetched.speed, fetched.name);

    close(fd);
    return 0;
}

