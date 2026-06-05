#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

int main(){
char str[50];
printf("enter the values val1,val2\n");
scanf(" %s",str);
int fd=open("/dev/work_queue",O_RDWR);
write(fd,str,sizeof(str));

}
