#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>  // malloc and free

int main()
{
    int fd;
    char *write_buf;
    char *read_buf;
    ssize_t n;
    size_t buf_size = 100;

    // Dynamically allocate buffers
    write_buf = (char *)malloc(buf_size);
    if (!write_buf) {
        perror("malloc failed for write_buf");
        return 1;
    }

    read_buf = (char *)malloc(buf_size);
    if (!read_buf) {
        perror("malloc failed for read_buf");
        free(write_buf);
        return 1;
    }

    // Open device
    fd = open("/dev/malloc_basic_char", O_RDWR);
    if (fd < 0) {
        perror("Error opening device");
        free(write_buf);
        free(read_buf);
        return 1;
    }

    printf("Enter operation (e.g., 10 20 add): ");
    if (!fgets(write_buf, buf_size, stdin)) {
        perror("fgets failed");
        free(write_buf);
        free(read_buf);
        close(fd);
        return 1;
    }

    // Remove newline
    write_buf[strcspn(write_buf, "\n")] = 0;

    if (write(fd, write_buf, strlen(write_buf)) < 0) {
        perror("write failed");
        free(write_buf);
        free(read_buf);
        close(fd);
        return 1;
    }

    n = read(fd, read_buf, buf_size - 1);
    if (n > 0) {
        read_buf[n] = '\0';
        printf("Result: %s", read_buf);
    } else {
        printf("Invalid operation or division by zero\n");
    }

    free(write_buf);
    free(read_buf);
    close(fd);

    return 0;
}
