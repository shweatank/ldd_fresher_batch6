#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int fd;
    char value[10];

    fd = open("/dev/pwm_led", O_WRONLY);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    while (1) {

        printf("\nEnter brightness (0-100): ");
        scanf("%s", value);

        write(fd, value, strlen(value));

    }

    close(fd);

    return 0;
}
