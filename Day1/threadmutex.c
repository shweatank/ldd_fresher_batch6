#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
pthread_t t1,t2;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
int var=0;
void *fun1(void* args)
{
	while(1)
	{
	pthread_mutex_lock(&mutex);
	var++;
	if(var>10)
        {
                  pthread_mutex_unlock(&mutex);
                  break;
        }
	printf("%d\n",var);
	pthread_mutex_unlock(&mutex);
	}
        pthread_exit(NULL);	
}
void *fun2(void* args)
{
        while(1)
	{
	pthread_mutex_lock(&mutex);
	printf("%d\n",var);
	var++;
	if(var>20)
	{
	          pthread_mutex_unlock(&mutex);
                  break;
	}
	pthread_mutex_unlock(&mutex);
	
	}
        pthread_exit(NULL);
}

int main()
{
  pthread_mutex_init(&mutex,NULL);
  pthread_create(&t1,NULL,fun1,NULL);
  pthread_create(&t2,NULL,fun2,NULL);
  pthread_join(t1,NULL);
  pthread_join(t2,NULL);
  pthread_mutex_destroy(&mutex);
  pthread_exit(NULL); 
}
