#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

struct calc {
    int a;
    int b;
    int result;
};

#define IOCTL_MAGIC 'C'
#define IOCTL_ADD _IOWR(IOCTL_MAGIC, 1, struct calc)
#define IOCTL_SUB _IOWR(IOCTL_MAGIC, 2, struct calc)
#define IOCTL_MUL _IOWR(IOCTL_MAGIC, 3, struct calc)
#define IOCTL_DIV _IOWR(IOCTL_MAGIC, 4, struct calc)

// Changed return type and parameter usage
static unsigned long op_to_ioctl(const char *p) {
    if (strcmp(p, "add") == 0) return IOCTL_ADD;
    if (strcmp(p, "sub") == 0) return IOCTL_SUB;
    if (strcmp(p, "mul") == 0) return IOCTL_MUL;
    if (strcmp(p, "div") == 0) return IOCTL_DIV;
    return 0; // Fallback
}

int main(int argc, char *argv[]) {
    int fd;
    if (argc != 4) {
        printf("Usage: %s <num1> <num2> <add|sub|mul|div>\n", argv[0]);
        return 0;
    }

    struct calc obj;
    obj.a = atoi(argv[1]);
    obj.b = atoi(argv[2]);
    
    // Store the actual IOCTL command number
    unsigned long cmd = op_to_ioctl(argv[3]);
    if (cmd == 0) {
        fprintf(stderr, "Invalid operation: %s\n", argv[3]);
        return 1;
    }

    fd = open("/dev/basic_ioctl", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Fixed printf: printing the string argv[3] instead of the long command
    printf("User: Sending %d %s %d to kernel\n", obj.a, argv[3], obj.b);

    if (ioctl(fd, cmd, &obj) < 0) {
        perror("ioctl");
        close(fd);
        return 1;
    }

    printf("User: got back %d from kernel\n", obj.result);

    close(fd);
    return 0;
}

