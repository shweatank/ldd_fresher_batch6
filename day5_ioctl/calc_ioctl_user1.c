
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdlib.h>

#define IOCTL_MAGIC 'C'

struct calc_data{
int a;
int b;
char op;
int result;
};

#define ADD _IOWR(IOCTL_MAGIC,1,struct calc_data)
#define SUB _IOWR(IOCTL_MAGIC,2,struct calc_data)
#define MUL _IOWR(IOCTL_MAGIC,3,struct calc_data)
#define DEV _IOWR(IOCTL_MAGIC,4,struct calc_data)

int main(int argc, char *argv[]){
int fd,cmd;
struct calc_data data;

fd=open("/dev/calc_ioctl",O_RDWR);
if(fd<0){
perror("open");
return 1;
}

data.a=atoi(argv[1]);
data.b=atoi(argv[2]);
data.op=argv[3][0];

switch(data.op)
{
case '+': cmd=ADD;
        break;
case '-': cmd=SUB;
        break;
case 'x': cmd=MUL;
        break;
case '/':cmd=DEV;
        break;
default: printf("Invalid operator!\n");
        return 1;
}
printf("User: sending %d %d and %c to kernel!\n",data.a,data.b,data.op);
ioctl(fd,cmd,&data);
printf("THe result: %d\n",data.result);
close(fd);
}
