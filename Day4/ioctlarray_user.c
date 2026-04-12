// user.c

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

struct arr_data {
    int size;
    int arr[100];
    int sum;
    float avg;
};

#define MY_IOCTL_MAGIC 'k'
#define ARRAY_OP _IOWR(MY_IOCTL_MAGIC, 1, struct arr_data)

int main()
{
    int fd = open("/dev/mydevice", O_RDWR);
    struct arr_data data;

    data.size = 5;
    data.arr[0] = 10;
    data.arr[1] = 20;
    data.arr[2] = 30;
    data.arr[3] = 40;
    data.arr[4] = 50;

    ioctl(fd, ARRAY_OP, &data);

    printf("Sum = %d\n", data.sum);
    printf("Average = %.2f\n", data.avg);

    close(fd);
    return 0;
}
