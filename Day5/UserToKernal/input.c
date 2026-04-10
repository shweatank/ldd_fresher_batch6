#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <num1> <num2> <operation>\n", argv[0]);
        return 1;
    }
    int fd = open("/dev/add_char", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }
    char buf[100];
    snprintf(buf, sizeof(buf), "%s %s %s", argv[1], argv[2], argv[3]);

    if (write(fd, buf, strlen(buf)) < 0) {
        perror("Failed to write to device");
    }

    close(fd);
}

