#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

struct CALC {
        int a;
        int b;
        long long int res;
};

static unsigned long op_to_ioctl(const char *op);
static struct CALC cal_req;


#define IOCTL_MAGIC 'A'
#define CALC_IOC_ADD _IOWR(IOCTL_MAGIC, 1, struct CALC)
#define CALC_IOC_SUB _IOWR(IOCTL_MAGIC, 2, struct CALC)
#define CALC_IOC_MUL _IOWR(IOCTL_MAGIC, 3, struct CALC)
#define CALC_IOC_DIV _IOWR(IOCTL_MAGIC, 4, struct CALC)
#define CALC_GET_RES _IOWR(IOCTL_MAGIC, 5,int)

static unsigned long op_to_ioctl(const char *op)
{
	if(!strcmp(op,"ADD")) return CALC_IOC_ADD; 
	if(!strcmp(op,"SUB")) return CALC_IOC_SUB; 
	if(!strcmp(op,"MUL")) return CALC_IOC_MUL; 
	if(!strcmp(op,"DIV")) return CALC_IOC_DIV;
       return 0;	
}

int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		printf("Usage : executable Num1 Num2 OPERATION (eg : ./a.out 2 3 ADD)\n");
		return 0;
	}
	int fd;
	
	cal_req.a = atoi(argv[1]);
	cal_req.b = atoi(argv[2]);


	fd = open("/dev/calc_driver", O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}
	//printf("User: sending %d to kernel\n",value);

	unsigned long cmd = op_to_ioctl(argv[3]);
	if(cmd != 0){
	  ioctl(fd, cmd, &cal_req);
	  printf("User: got back %lld from kernel\n",cal_req.res);
	}

	while(1){
	  int n = 0;
	  printf("1.Get Result of operation\n2.Exit\nEnter the choice : ");
          scanf("%d",&n);	
	if(n == 1){
		int res;
	     ioctl(fd, CALC_GET_RES, &res);
	     printf("The result fetched from ioctl for the last operation is %d\n",res);
	}
	if(n == 2){
	   break;
	}
	}

	close(fd);
	return 0;
}
