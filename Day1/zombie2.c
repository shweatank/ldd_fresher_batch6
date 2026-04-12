#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>

int main()
{
  int res=fork();
  if(res)
  {
   int status;
    printf("ppid=%d\n",getpid());
    printf("parent exiting\n");
   int chpid= wait(&status);
   printf("chpid=%d status=%d \n",chpid,(status>>8)&0xff);
  }
  else if(res==0)
  {
    sleep(5);
    printf("in child pid=%d\n",getpid());
    printf("child exiting\n");
    exit(16);
  }
}
