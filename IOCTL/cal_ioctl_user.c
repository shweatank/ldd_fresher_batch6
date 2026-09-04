#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#define IOCTL_MAGIC 'G'

#define CALC_IOC_ADD _IOWR(IOCTL_MAGIC , 1 , struct data)
#define CALC_IOC_SUB _IOWR(IOCTL_MAGIC , 2 , struct data)
#define CALC_IOC_MUL _IOWR(IOCTL_MAGIC , 3 , struct data)
#define CALC_IOC_DIV _IOWR(IOCTL_MAGIC , 4 , struct data)
#define CALC_GET_VALUE _IOR(IOCTL_MAGIC , 5 , int)

struct data {
    int num1;
    int num2;
    int result;
    char op;
};

int main()
{
    int fd = open("/dev/cal_ioctl_kernel", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct data temp;
    int value;

    printf("Enter the operation (+, -, *, /): ");
    scanf(" %c", &temp.op);   // space important

    printf("Enter the two numbers: ");
    scanf("%d %d", &temp.num1, &temp.num2);

    switch (temp.op)
    {
        case '+':
            ioctl(fd, CALC_IOC_ADD, &temp);
            break;
        case '-':
            ioctl(fd, CALC_IOC_SUB, &temp);
            break;
        case '*':
            ioctl(fd, CALC_IOC_MUL, &temp);
            break;
        case '/':
            ioctl(fd, CALC_IOC_DIV, &temp);
            break;
        default:
            printf("Invalid operator\n");
            close(fd);
            return 1;
    }

    ioctl(fd, CALC_GET_VALUE, &value); 
    

    printf("Result from kernel: %d\n", value);

    close(fd);
    return 0;
}
