#include<stdio.h>
#include<pthread.h>
int counter = 0;
pthread_mutex_t lock;

void* fun(void* arg){
  for(int i=0;i<100000;i++){
         pthread_mutex_lock(&lock);
      	  counter++;
         pthread_mutex_unlock(&lock);
  }
  return NULL;
}
int main(){
  pthread_t th1,th2;	
  pthread_mutex_init(&lock,NULL); //initialize the mutex

  pthread_create(&th1,NULL,fun,NULL);
  pthread_create(&th2,NULL,fun,NULL);
  
  pthread_join(th1,NULL);
  pthread_join(th2,NULL);
  
  pthread_mutex_destroy(&lock);
  printf("Final COunter: %d\n",counter);  
}
