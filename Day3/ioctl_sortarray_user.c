#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct array_data {
    int size;
    int arr[50];
};

#define MY_IOCTL_MAGIC 'k'
#define SORT_ARRAY _IOWR(MY_IOCTL_MAGIC, 1, struct array_data)

int main()
{
    int fd;
    struct array_data data;
    int i;

    fd = open("/dev/basic_char", O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    printf("file successfully opened\n");

    printf("Enter number of elements: ");
    scanf("%d", &data.size);

    printf("Enter elements:\n");
    for (i = 0; i < data.size; i++)
        scanf("%d", &data.arr[i]);

    printf("Before sorting:\n");
    for (i = 0; i < data.size; i++)
        printf("%d ", data.arr[i]);
    printf("\n");

    ioctl(fd, SORT_ARRAY, &data);

    printf("After sorting:\n");
    for (i = 0; i < data.size; i++)
        printf("%d ", data.arr[i]);
    printf("\n");

    close(fd);

    return 0;
}
