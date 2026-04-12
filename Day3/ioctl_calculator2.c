//basic_ioctl_user.c
//Minimal user program calling ioctl

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct calculator{
   int num1;
   int num2;
   char opr;
   int result;
}CAL;

#define IOCTL_MAGIC 'D'
#define CALC_IOC_ADD _IOWR(IOCTL_MAGIC, 1,CAL)
#define CALC_IOC_SUB _IOWR(IOCTL_MAGIC, 2,CAL)
#define CALC_IOC_MUL _IOWR(IOCTL_MAGIC, 3,CAL)
#define CALC_IOC_DIV _IOWR(IOCTL_MAGIC, 4,CAL)
#define CALC_IOC_MOD _IOWR(IOCTL_MAGIC, 5,CAL)

int main(int argc,char *argv[])
{
	int fd;
	CAL C;
	unsigned long cmd;
	fd = open("/dev/basic_ioctl", O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}
        
	C.num1=atoi(argv[1]);
	C.num2=atoi(argv[2]);
	C.opr=argv[3][0];
	printf("User: sending num1=%d to kernel\n", C.num1);
        printf("User: sending num2=%d to kernel\n", C.num2);
	printf("User: sending num1=%c to kernel\n", C.opr);

        switch(C.opr)
	{
	  case '+': cmd=CALC_IOC_ADD;
		    break;
	  case '-':cmd=CALC_IOC_SUB;
                    break;

	  case 'x':cmd=CALC_IOC_MUL;
                    break;

	  case '/':cmd=CALC_IOC_DIV;
                    break;
	  case '%':cmd=CALC_IOC_ADD;
                    break;
          default:
		   printf("invalid argument\n");
		   return 1;
	}
	ioctl(fd, cmd , &C);

    
	printf("User: got back %d %c %d result =%d from kernel\n", C.num1,C.opr,C.num2,C.result);
       
	close(fd);
	return 0;
}
