#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MAGIC_NUM 'C'
#define IOCTL_KEY_SCAN _IOWR(MAGIC_NUM,1,int)

int main(){
int op;
int fd;
fd=open("/dev/key",O_RDWR);
if(fd<0){
perror("open");
return 0;
printf("enter the operation 1) addition 2)sub 3)mul 4)div\n");
scanf(" %d",&op);
ioctl(fd,IOCTL_KEY_SCAN,&op);
printf("result=%d\n",op);
}

}
