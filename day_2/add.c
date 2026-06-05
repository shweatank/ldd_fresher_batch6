#include<stdio.h>
#include<fcntl.h>
#include<sys/types.h>
#include<stdlib.h>
#include<unistd.h>

int main(){

int val=0;
int fd=open("/dev/basic_char",O_RDWR);
write(fd,"9,9,+",6);
read(fd,&val,1);
printf("ANSWER=%d\n",val);

}

