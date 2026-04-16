#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>

int main()
{
int fd;
char write_buf[100];
char read_buf[100];

fd=open("/proc/proc_calc",O_RDWR);
if(fd<0)
{
perror("open");
return -1;
}
printf("Enter the expression (ex: 10 20 +): ");
fgets(write_buf,sizeof(write_buf),stdin);
write(fd,write_buf,strlen(write_buf));
lseek(fd,0,SEEK_SET);//reset offset
read(fd,read_buf,sizeof(read_buf));
printf("Result: %s\n",read_buf);
close(fd);
}
