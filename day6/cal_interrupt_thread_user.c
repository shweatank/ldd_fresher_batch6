#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

#define BUF_SIZE 1024
int main()
{
	int fd;
	int num1,num2;
	char op;
	char buf[BUF_SIZE];
	fd=open("/dev/calc_irq",O_RDWR);
	if(fd==-1)
	{
		perror("open");
		return 1;
	}
	printf("enter the two numbers:\n");
	scanf("%d %d",&num1,&num2);
	printf("enter the operator:\n");
	scanf(" %c",&op);
	sprintf(buf,"%d %d %c",num1,num2,op);
	write(fd,buf,strlen(buf));
	char str[100];
	int n;
        if((n=read(fd,str,sizeof(str)-1))<0)
        {
                perror("reaad");
                return 1;
        }
	str[n]='\0';
	printf("Result :%s\n",str);
        close(fd);

}
