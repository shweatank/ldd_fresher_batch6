#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct sum_data {
    int value;
    int result;
};

#define MY_IOCTL_MAGIC 'k'
#define SET_SUM _IOW(MY_IOCTL_MAGIC, 1, struct sum_data)
#define GET_SUM _IOR(MY_IOCTL_MAGIC, 2, struct sum_data)

int main()
{
    int fd;
    struct sum_data data;

    fd = open("/dev/sumdev", O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    while (1)
    {
        printf("Enter value to add (-1 to stop): ");
        scanf("%d", &data.value);

        if (data.value == -1)
            break;

        ioctl(fd, SET_SUM, &data);
    }

    ioctl(fd, GET_SUM, &data);

    printf("Final Running Sum = %d\n", data.result);

    close(fd);
    return 0;
}
