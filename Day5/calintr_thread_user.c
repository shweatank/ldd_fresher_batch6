#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
int main()
{
  int fd=open("/dev/calc_irq",O_RDWR);
  if(fd<0)
  {
   perror("open");
   return 1;
  }
  int num1,num2;
  char buf[100],opr;
  printf("Enter two numbers:\n");
  scanf(" %d %d",&num1,&num2);
  printf("Enter the operation: \n");
  printf("Add= + sub= - div= / mul= *\n");
  scanf(" %c",&opr);
  sprintf(buf," %d %d %c", num1, num2, opr);
  write(fd,buf,strlen(buf));
 
  char res[10];
  int n;
  n=read(fd,res,sizeof(res)-1);
  if(n>0)
  {
	  res[n]='\0';
	  printf("Result=%s\n",res);
  }
  else
  {
	  perror("read");
  }
  close(fd);
  return 0;
}
