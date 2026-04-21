#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    int fd;
    int num1, num2;
    char opr;
    char buf[100];
    char res[100];
    int n;

    fd = open("/dev/basic_char", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf("Enter two numbers:\n");
    scanf("%d %d", &num1, &num2);

    printf("Enter operation (+ - * /): ");
    scanf(" %c", &opr);

    /* send structured string to kernel */
    snprintf(buf, sizeof(buf), "%d %c %d", num1, opr, num2);

    if (write(fd, buf, strlen(buf)) < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    /* read result */
    n = read(fd, res, sizeof(res) - 1);
    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    res[n] = '\0';
    printf("Result = %s\n", res);

    close(fd);
    return 0;
}

