#include<stdio.h>
#include<unistd.h>
#include<signal.h>
int main(){
 while(1){
   printf("Process startd: pid: %d\n",getpid());
   sleep(4);
 } 
}
