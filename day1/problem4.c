#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
pthread_t tid1,tid2;
int count=0;
void *increment(void *args)
{
	for(int i=0;i<100000;i++)
		count++;
}
int main()
{
	pthread_create(&tid1,NULL,increment,NULL);
	pthread_create(&tid2,NULL,increment,NULL);
	pthread_join(tid1,NULL);
	pthread_join(tid2,NULL);
	printf("Final result:%d\n",count);
}
