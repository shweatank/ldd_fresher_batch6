//basic_ioctl_user.c
//Minimal user program calling ioctl

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

typedef struct calculator{
   int num1;
   int num2;
   char opr;
   int result;
}CAL;

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC, 1,CAL)

int main()
{
	int fd;
	CAL C;
	printf("Enter the two numbers\n");
	scanf("%d %d",&C.num1,&C.num2);
	 
        printf("Enter the operator\n");
	scanf(" %c",&C.opr);

	fd = open("/dev/basic_ioctl", O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}

	printf("User: sending num1=%d to kernel\n", C.num1);
        printf("User: sending num2=%d to kernel\n", C.num2);
	printf("User: sending num1=%c to kernel\n", C.opr);

	ioctl(fd, IOCTL_SET_VALUE, &C);

	printf("User: got back %d %c %d result =%d from kernel\n", C.num1,C.opr,C.num2,C.result);
       
	close(fd);
	return 0;
}
