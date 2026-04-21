#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

struct calculator {
    int num1;
    int num2;
    char opr;
    int result;
};

int main()
{
    int fd;
    struct calculator cal;
    char output[128];
    int n;

    fd = open("/proc/proc_demo", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf("Enter num1 num2:\n");
    scanf("%d %d", &cal.num1, &cal.num2);

    printf("Enter operator (+ - * /): ");
    scanf(" %c", &cal.opr);

    /* send struct to kernel */
    if (write(fd, &cal, sizeof(cal)) < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    /* read result */
    lseek(fd, 0, SEEK_SET);

    n = read(fd, output, sizeof(output) - 1);
    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    output[n] = '\0';
    printf("%s", output);

    close(fd);
    return 0;
}
