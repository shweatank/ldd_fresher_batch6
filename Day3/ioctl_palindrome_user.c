#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define BUFSIZE 256

struct pal_data {
    char str[BUFSIZE];
    char result[32];
};

#define MY_IOCTL_MAGIC 'k'
#define SET_STRING _IOW(MY_IOCTL_MAGIC, 1, struct pal_data)
#define GET_RESULT _IOR(MY_IOCTL_MAGIC, 2, struct pal_data)

int main()
{
    int fd;
    struct pal_data data;

    fd = open("/dev/basic_char", O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    printf("Enter string: ");
    fgets(data.str, BUFSIZE, stdin);
    data.str[strcspn(data.str, "\n")] = '\0';

    ioctl(fd, SET_STRING, &data);
    ioctl(fd, GET_RESULT, &data);

    printf("Result: %s\n", data.result);

    close(fd);
    return 0;
}
