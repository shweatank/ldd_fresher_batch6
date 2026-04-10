#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    int fd;
    char *write_buf;
    char *read_buf;
    ssize_t n;
    size_t buf_size = 100;

    write_buf = (char *)malloc(buf_size);
    read_buf = (char *)malloc(buf_size);

    if (!write_buf || !read_buf)
    {
        perror("malloc failed");
        return 1;
    }

    fd = open("/dev/palin_basic_char", O_RDWR);
    if (fd < 0)
    {
        perror("Error opening device");
        free(write_buf);
        free(read_buf);
        return 1;
    }

    printf("Enter string: ");
    fgets(write_buf, buf_size, stdin);

    write_buf[strcspn(write_buf, "\n")] = 0;

    write(fd, write_buf, strlen(write_buf));

    n = read(fd, read_buf, buf_size - 1);

    if (n > 0)
    {
        read_buf[n] = '\0';
        printf("Result: %s", read_buf);
    }
    else
    {
        printf("Error reading result\n");
    }

    free(write_buf);
    free(read_buf);
    close(fd);

    return 0;
}
