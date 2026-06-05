#include<stdio.h>
#include<fcntl.h>
#include<sys/types.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

int main(){

char buf[50];
char p[100];
printf("enter the val1,val2,operator\n");
scanf(" %s",p);
int fd=open("/dev/basic_char",O_RDWR);
write(fd,p,8);
read(fd,&buf,sizeof(buf));
int len=strlen(buf);
buf[len]='\0';
printf("ANSWER=%s\n",buf);

}

