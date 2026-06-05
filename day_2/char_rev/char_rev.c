#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

int main(){
char str[50];
char buf[50];
printf("enter the string\n");
scanf("%[^\n]",str);
int fd=open("/dev/basic_char",O_RDWR);
write(fd,str,sizeof(str));
read(fd,&buf,sizeof(buf));
int len=strlen(buf);
buf[len]='\0';
printf("reverse string=%s\n",buf);

}

