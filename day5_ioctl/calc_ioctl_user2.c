#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdlib.h>

#define IOCTL_MAGIC 'D'

struct calc_data{
int a;
int b;
char op;

};

#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct calc_data)
#define IOCTL_GET_VALUE _IOWR(IOCTL_MAGIC,2,int)


int main(){
int fd,cmd,result;
struct calc_data data;

fd=open("/dev/calc_ioctl",O_RDWR);
if(fd<0){
perror("open");
return 1;
}
printf("Enter the two operands: ");
scanf("%d %d",&data.a,&data.b);

printf("Enter the operator:");
scanf(" %c",&data.op);

printf("User: sending %d %d and %c to kernel!\n",data.a,data.b,data.op);
ioctl(fd,IOCTL_SET_VALUE,&data);
ioctl(fd,IOCTL_GET_VALUE,&result);

printf("THe result: %d\n",result);
close(fd);
}



