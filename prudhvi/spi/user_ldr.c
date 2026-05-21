#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE "/dev/ldr_adc"

int main()
{
    int fd;
    char buffer[32];

    fd = open(DEVICE, O_RDONLY);

    if (fd < 0) {

        perror("Device open failed");

        return -1;
    }

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        read(fd, buffer, sizeof(buffer));

        printf("%s", buffer);
        sleep(1);
        lseek(fd, 0, SEEK_SET);
    }

    close(fd);

    return 0;
}
