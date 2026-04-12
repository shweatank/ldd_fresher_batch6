#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
struct number_data {
    int num;
    char result[10];
};

#define MY_IOCTL_MAGIC 'k'
#define CHECK_EVEN_ODD _IOWR(MY_IOCTL_MAGIC, 1, struct number_data)

int main()
{
    int fd = open("/dev/basic_char", O_RDWR);
    struct number_data data;

    printf("Enter a Number\n");
    scanf("%d",&data.num);

    ioctl(fd, CHECK_EVEN_ODD, &data);

    printf("Result: %s\n", data.result);

    close(fd);
    return 0;
}
