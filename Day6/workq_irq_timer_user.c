#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd;
    char wbuf[] = "generate data";
    char rbuf[100];

    fd = open("/dev/mydevice", O_RDWR);

    write(fd, wbuf, sizeof(wbuf));

    printf("Waiting for data...\n");

    read(fd, rbuf, sizeof(rbuf));

    printf("Received: %s\n", rbuf);

    close(fd);
    return 0;
}
