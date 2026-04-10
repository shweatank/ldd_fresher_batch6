#include<stdio.h>
#include<pthread.h>
int counter = 0;
void* fun(void* arg){
  for(int i=0;i<100000;i++){
     counter++;
  }
  return NULL;
}
int main(){
  pthread_t th1,th2;	
  pthread_create(&th1,NULL,fun,NULL);
  pthread_create(&th2,NULL,fun,NULL);
  
  pthread_join(th1,NULL);
  pthread_join(th2,NULL);
 
  printf("Final COunter: %d\n",counter);  
}
