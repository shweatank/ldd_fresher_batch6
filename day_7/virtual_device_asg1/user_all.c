#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(){
int fd;
char str[50];
printf("enter the string\n");
scanf(" %s",str);
fd=("/dev/user",O_RWDR);
write(fd,str,sizeof(str));
read(fd,str,sizeof(str));
printf("result= %s ",str);

}
