#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
int main(){
 int fd=open("/dev/waitq_basic",O_RDWR);
 char buf[10]="Hello";
 write(fd,buf,10);
 close(fd);
}

