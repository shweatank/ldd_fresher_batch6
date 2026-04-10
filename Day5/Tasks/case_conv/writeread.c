#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
int main() {
    int fd = open("/dev/add_char", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    char *command = "RahuL";

    if (write(fd, command, strlen(command)) < 0) {
        perror("write failed");
        close(fd);
        return 1;
    }

    lseek(fd, 0, SEEK_SET);

    char buf[100] = {0};

    int n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0) {
        perror("read failed");
        close(fd);
        return 1;
    }

    printf("Result from Kernel: %s\n", buf);

    close(fd);
    return 0;
}
