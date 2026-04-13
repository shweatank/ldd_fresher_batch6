#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdlib.h>

#define IOCTL_MAGIC 'C'
#define IOCTL_ARRAY_SUM _IOWR(IOCTL_MAGIC,1,struct array_data)
#define MAX_SIZE 100

struct array_data{
int a[MAX_SIZE];
int size;
int sum;
int avg;
};

int main(){
int fd,cmd;
struct array_data data;

fd=open("/dev/array_ioctl",O_RDWR);
if(fd<0){
perror("open");
return 1;
}

printf("Enter the size of the array: ");
scanf("%d",&data.size);
printf("Enter the array elements: ");
for(int i=0;i<data.size;i++){
scanf("%d",&data.a[i]);
}

ioctl(fd,IOCTL_ARRAY_SUM,&data);
printf("The sum of array elements: %d\n",data.sum);
printf("THe average of  array elements: %d\n",data.avg);
close(fd);
}
