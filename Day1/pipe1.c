#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
int main(){
  int a[2];	
  pipe(a);
  printf("%d %d\n",a[0],a[1]);
}
