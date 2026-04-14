#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include "dynamic_ioctl.h"

int main() {
    int fd = open("/dev/dyn_buf_dev", O_RDWR);
    size_t size = 100;
    char write_msg[] = "Hello from Dynamic Buffer!";
    char read_buf[100] = {0};

    // 1. Resize buffer to 100 bytes
    ioctl(fd, SET_BUFFER_SIZE, &size);

    // 2. Write data
    write(fd, write_msg, strlen(write_msg));

    // 3. Read back (resetting file pointer by closing/reopening or lseek)
    lseek(fd, 0, SEEK_SET);
    read(fd, read_buf, sizeof(read_buf));
    
    printf("Read from kernel: %s\n", read_buf);

    close(fd);
    return 0;
}

