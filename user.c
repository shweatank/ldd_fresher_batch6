#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int fd;
    char write_buf[100];
    char read_buf[100];
    ssize_t n;

    fd = open("/dev/basic_char", O_RDWR);
    if (fd < 0) {
        perror("Error opening device");
        return 1;
    }

    printf("Enter operation (e.g., 10 20 add): ");
    fgets(write_buf, sizeof(write_buf), stdin);

    write(fd, write_buf, strlen(write_buf));

    n = read(fd, read_buf, sizeof(read_buf) - 1);
    if (n > 0) {
        read_buf[n] = '\0';
        printf("Result: %s", read_buf);
    } else {
        printf("Invalid operation or division by zero\n");
    }

    close(fd);
    return 0;
}
