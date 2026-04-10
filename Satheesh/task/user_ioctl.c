#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
/*struct st
{
	int a,b,c;
};
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct st)
*/


#define CALC_IOC_MAGIC 'C'
//#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)

struct calc
{
	int a,b;
};
static int result;
#define CALC_IOC_WADD _IOW(CALC_IOC_MAGIC,1,struct calc)
#define CALC_IOC_RADD _IOR(CALC_IOC_MAGIC,2,int)

#define CALC_IOC_WSUB _IOW(CALC_IOC_MAGIC,3,struct calc)
#define CALC_IOC_RSUB _IOR(CALC_IOC_MAGIC,4,int)

#define CALC_IOC_WMUL _IOW(CALC_IOC_MAGIC,5,struct calc)
#define CALC_IOC_RMUL _IOR(CALC_IOC_MAGIC,6,int)

#define CALC_IOC_WDIV _IOW(CALC_IOC_MAGIC,7,struct calc)
#define CALC_IOC_RDIV _IOR(CALC_IOC_MAGIC,8,int)
int do_op(char **argv)
{
	int num1=atoi(argv[1]);
	int num2=atoi(argv[2]);
	int result;
	struct calc obj;
	obj.a=num1,obj.b=num2;
	int fd=open("/dev/basic_ioctl",O_RDWR);
	if(strcmp(argv[3],"sum")==0)
	{
		ioctl(fd,CALC_IOC_WADD,&obj);
		ioctl(fd,CALC_IOC_RADD,&result);
		return result;
	}else if(strcmp(argv[3],"sub")==0)
	{
		ioctl(fd,CALC_IOC_WSUB,&obj);
		ioctl(fd,CALC_IOC_RSUB,&result);
		return result;
	}else if(strcmp(argv[3],"mul")==0)
	{
		ioctl(fd,CALC_IOC_WMUL,&obj);
		ioctl(fd,CALC_IOC_RMUL,&result);
		return result;
	}else if(strcmp(argv[3],"div")==0)
	{
		ioctl(fd,CALC_IOC_WDIV,&obj);
		ioctl(fd,CALC_IOC_RDIV,&result);
		return result;
	}else {
		printf("Enter correct operation\n");
		close(fd);
		return -1;
	}
close(fd);
}

int main(int argc,char **argv)
{
	if(argc<4)
	{
		printf("Usage:./a.out num1 num2 op\n");
		return 1;
	}
	int result=do_op(argv);
	printf("Result: %d\n",result);	
	return 0;
	
}	
