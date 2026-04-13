/*user program to send input values through structure and get out put through ioctl */
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/ioctl.h>

#define IOCTL_MAGIC 'A'
#define SET_DATA _IOW(IOCTL_MAGIC,1,struct calc_data *)
#define GET_DATA _IOR(IOCTL_MAGIC,2,struct calc_data *)

struct calc_data{
int a;
int b;
char op;
int result;
};


int main(){
struct calc_data data;

/* opening file to write */
int fd=open("/dev/basic_ioctl",O_RDWR);
if(fd<0){
perror("Error in opening file!\n");
return 1;
}

printf("Enter operand1, operator and operand2: ");
scanf("%d %c %d",&data.a,&data.op,&data.b);

ioctl(fd,SET_DATA,&data);//send data to driver

printf("NOw press a key to trigger IRQ..\n");
getchar(); getchar(); //wait for key press

ioctl(fd,GET_DATA,&data);
printf("Result from kernel : %d\n",data.result);

close(fd);
}


