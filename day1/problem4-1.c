#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
pthread_t tid1,tid2;
int count=0;
pthread_mutex_t lock;
void *increment(void *args)
{
	for(int i=0;i<100000;i++)
	{
		pthread_mutex_lock(&lock);
		count++;
	        pthread_mutex_unlock(&lock);
	}
	pthread_exit(NULL);
}
int main()
{
	pthread_mutex_init(&lock,NULL);
	pthread_create(&tid1,NULL,increment,NULL);
	pthread_create(&tid2,NULL,increment,NULL);
	pthread_join(tid1,NULL);
	pthread_join(tid2,NULL);
	pthread_mutex_destroy(&lock);
	printf("Final result:%d\n",count);
}
