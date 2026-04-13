/*
// incrementing without mutex lock
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

int count=0;

void* increment(void* arg){
for(int i=0;i<100000;i++){
count++;
}
}

int main(){
pthread_t thread1,thread2;

pthread_create(&thread1,NULL,increment,NULL);
pthread_create(&thread2,NULL,increment,NULL);

pthread_join(thread1,NULL);
pthread_join(thread2,NULL);

printf("Counter after incrementing: %d\n",count);
}
*/ 


//with mutex lock
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

int count=0;
pthread_mutex_t lock;

void* increment(void* arg){
pthread_mutex_lock(&lock);
for(int i=0;i<100;i++){
printf("thread1 : %d\n",count);
count++;
printf("thread2: %d\n",count);
}
pthread_mutex_unlock(&lock);
}

int main(){
pthread_t thread1,thread2;

pthread_mutex_init(&lock,NULL);

pthread_create(&thread1,NULL,increment,NULL);
pthread_create(&thread2,NULL,increment,NULL);

pthread_join(thread1,NULL);
pthread_join(thread2,NULL);

pthread_mutex_destroy(&lock);
printf("Final Counter: %d\n",count);
}

