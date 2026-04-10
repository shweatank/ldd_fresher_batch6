//MInimal user program calling ioctl

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
struct calc{
  int a;
  int b;
  char op[10];
  int result;
};
#define IOCTL_MAGIC 'B'
#define IOCTL_CALC _IOWR(IOCTL_MAGIC , 1, struct calc)
int main(int argc , char *argv[]){
//int main(){
  int fd;
  if(argc !=4){
    puts("./a.out <num1> <num2> <Operation>\n"); return 0;
  }
  struct calc obj;
  obj.a=atoi(argv[1]);
  obj.b=atoi(argv[2]);
  strcpy(obj.op,argv[3]);


  /*struct calc obj;
  obj.a=23;
  obj.b=47;
  strcpy(obj.op,"add");*/
  fd=open("/dev/basic_ioctl",O_RDWR);
  if(fd<0){
   perror("open");
   return 1;
  }

  printf("User: Sending %d %s %d to kernal\n",obj.a,obj.op,obj.b);

  if(ioctl(fd,IOCTL_CALC,&obj)<0){
	perror("ioctl");
        close(fd);
        return 1;
  }

  printf("User: got back %d from kernal\n",obj.result);

  close(fd);
  return 0;
}
