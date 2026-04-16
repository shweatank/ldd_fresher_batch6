#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

int main(void)
{
	int num1,num2;
	char op;
        int fd;
	char input[100],output[100];
        fd=open("/proc/proc_demo",O_RDWR);
        if(fd<0)
        {
                perror("open");
                return 1;
        }
	printf("enter the data:");
	scanf("%d %c %d",&num1,&op,&num2);
        sprintf(input,"%d %c %d",num1,op,num2);
	write(fd,input,strlen(input));
	lseek(fd,0,SEEK_SET);
	int n=read(fd,output,sizeof(output)-1);
	if(n>0)
	{
		output[n]='\0';
	        printf("Kernel output:%s\n",output);
	}
	else
	{
		printf("No data received\n");
	}
        close(fd);
        return 0;
}
