#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "array_ioctl.h"

int main() {
    int fd = open("/dev/array_dev", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    int my_numbers[] = {10, 20, 30, 40, 50};
    struct array_payload packet;

    packet.user_ptr = my_numbers;
    packet.size = 5;

    printf("Sending array to kernel...\n");
    if (ioctl(fd, PROCESS_ARRAY, &packet) < 0) {
        perror("ioctl");
    } else {
        printf("Kernel Results -> Sum: %ld, Avg: %d\n", packet.sum, packet.avg);
    }

    close(fd);
    return 0;
}

