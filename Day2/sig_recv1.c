#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
int main(int argc, char *argv[]){
 if(argc!=3){
   puts("<executable> <PID> <SIG>\n"); return 0;
 }
  kill(atoi(argv[1]),atoi(argv[2]));
}

