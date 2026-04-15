#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

int main(){
int fd;
char buf[100];
fd=open("/dev/log_device",O_RDWR);
if(fd<0)
{
perror("open");
return -1;
}

write(fd,"Hello kernel log1",18);
write(fd,"Hello kernel log2",18);
write(fd,"Hello kernel log3",18);

printf("Waiting for the data..\n");
read(fd,buf,sizeof(buf));
printf("LOGS: \n%s",buf);
close(fd);
}
