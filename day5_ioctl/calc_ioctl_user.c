//basic_ioctl_user.c
//minimal user program calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>

#define IOCTL_MAGIC 'A'

struct calc_data{
int a;
int b;
char op;
int result;
};

#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct calc_data)


int main(void){
int fd;
struct calc_data data;

fd=open("/dev/calc_ioctl",O_RDWR);
if(fd<0){
perror("open");
return 1;
}

printf("Enter the two operands and the operator: ");
scanf("%d %d %c",&data.a,&data.b,&data.op);


printf("User: sendind data to kernel\n");
ioctl(fd,IOCTL_SET_VALUE,&data);
printf("THe result: %d\n",data.result);
close(fd);
}


