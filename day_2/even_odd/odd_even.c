#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>

int main(){
int fd=open("/dev/basic_char",O_RWND);
char val[50];
char op[50];
printf("enter the value as the string\n");
scanf(" %s",val);
write(fd,val,strlen(val));

}
