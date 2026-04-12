#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
int main()
{
  int fd=open("/dev/basic_char",O_WRONLY);
  char num1,num2,buf[100],opr;
  printf("Enter two numbers:\n");
  scanf(" %c %c",&num1,&num2);
  printf("Enter the operation: \n");
  printf("Add= + sub= - div= / mul= *\n");
  scanf(" %c",&opr);
  snprintf(buf,sizeof(buf), "%c%c%c",num1,num2,opr);
  write(fd,buf,strlen(buf));
 
  close(fd);
  char res[10];
  int n;
  fd=open("/dev/basic_char",O_RDONLY);
  if(fd<0)
  {
	  perror("open");
	  return 1;
  }
  n=read(fd,res,sizeof(res)-1);
  if(n>0)
  {
	  res[n]='\0';
	  printf("Result=%s\n",res);
  }
  else
  {
	  perror("read");
	  close(fd);
	  return 1;
  }
  close(fd);
  return 0;
}
