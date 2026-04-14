//MInimal user program calling ioctl
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>

struct calc{
  int a;
  int b;
  char op[10];   // not used now, but kept for compatibility
  int result;
};

#define IOCTL_MAGIC 'B'
#define IOCTL_CALC _IOWR(IOCTL_MAGIC , 1, struct calc)

int main(int argc , char *argv[]){
  int fd;

  // Only 2 inputs needed now
  if(argc != 3){
    printf("Usage: %s <num1> <num2>\n", argv[0]);
    return 0;
  }

  struct calc obj;

  obj.a = atoi(argv[1]);
  obj.b = atoi(argv[2]);

  fd = open("/dev/basic_ioctl", O_RDWR);
  if(fd < 0){
    perror("open");
    return 1;
  }

  printf("User: Sending %d and %d to kernel\n", obj.a, obj.b);
  printf("NOTE: Operation is selected via keyboard (A/B/C/D)\n");

  if(ioctl(fd, IOCTL_CALC, &obj) < 0){
        perror("ioctl");
        close(fd);
        return 1;
  }

  printf("User: got result = %d from kernel\n", obj.result);

  close(fd);
  return 0;
}
