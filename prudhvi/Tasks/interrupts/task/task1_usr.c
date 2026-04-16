#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>


int main()
{
int fd=open("/dev/task1",O_RDWR);
if(fd<0)
{
perror("open");
return 1;
}
char s[30]="GENERATE..",r[30];
if(write(fd,s,strlen(s)+1)<0)
{
perror("write");
return 1;
}
printf("sended data to kernel waiting for recevie..\n");
if(read(fd,r,sizeof(r))<0)
{
perror("write");
return 1;
}
printf("recevied data from kernel...\n");
printf("%s\n",r);

close(fd);


}
