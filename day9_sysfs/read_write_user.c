#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

int main(){
int fd;
char buf[50];

fd=open("/dev/waitq_basic",O_RDWR);
if(fd<0)
{
perror("open");
return -1;
}
write(fd,"generate",8);
printf("Waiting for the data..\n");
read(fd,buf,sizeof(buf));
printf("Generated data : %s\n",buf);
close(fd);
}

