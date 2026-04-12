#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define BUF_SIZE 256

struct case_data {
    char str[BUF_SIZE];
    char result[BUF_SIZE];
};

#define MY_IOCTL_MAGIC 'k'
#define SET_STRING _IOW(MY_IOCTL_MAGIC, 1, struct case_data)
#define GET_RESULT _IOR(MY_IOCTL_MAGIC, 2, struct case_data)

int main()
{
    int fd;
    struct case_data data;

    fd = open("/dev/basic_char", O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    printf("Enter string: ");
    fgets(data.str, BUF_SIZE, stdin);
    data.str[strcspn(data.str, "\n")] = '\0';

    ioctl(fd, SET_STRING, &data);
    ioctl(fd, GET_RESULT, &data);

    printf("Converted string: %s\n", data.result);

    close(fd);
    return 0;
}
