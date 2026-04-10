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

    char *command = "madama";
    write(fd, command, strlen(command));

    // Seek back to the beginning of the "file" to read the result
    lseek(fd, 0, SEEK_SET);

    char buf[100];
    memset(buf, 0, sizeof(buf)); // Clear buffer
    read(fd, buf, sizeof(buf) - 1);

    printf("Result from Kernel: %s\n", buf);

    close(fd);
    return 0;
}

