#include<stdio.h>
#include <signal.h>
#include<unistd.h>
#include<stdlib.h>

int a=10,b=5;

//signalhandler format should be void signal_handler(int signal_number);
void add(int signum){
printf("Perfomred addition, Sum : %d\n",a+b);
}
void subtract(int signum){
printf("Perfomred substraction, Difference : %d\n",a-b);
}
void multiply(int signum){
printf("Perfomred multiplication, Product : %d\n",a*b);
}
void devide(int signum){
printf("Perfomred devision, Quotient : %d\n",a/b);
}
void terminate(int signum){
printf("program terminated successfully!\n");
exit(0);
}


int main(){
printf("get pid: %d\n",getpid());
signal(1,add);
signal(2,subtract);
signal(3,multiply);
signal(4,devide);
signal(5,terminate);
while(1); //using while(1) to keep the process alive
return 0;
}
