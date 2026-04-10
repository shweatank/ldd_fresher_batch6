#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
int main() {
    // Open with RDWR (Read and Write)
    int fd = open("/dev/add_char", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    // 1. Send the command to the driver
    char *command = "11 5 add";
    write(fd, command, strlen(command));

    // 2. Seek back to the beginning of the "file" to read the result
    lseek(fd, 0, SEEK_SET);

    // 3. Read the result back
    char buf[100];
    memset(buf, 0, sizeof(buf)); // Clear buffer
    read(fd, buf, sizeof(buf) - 1);

    printf("Result from Kernel: %s\n", buf);

    close(fd);
    return 0;
}

