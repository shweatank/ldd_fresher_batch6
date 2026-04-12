#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
pthread_t t1,t2;
int var=0;
void *fun1(void* args)
{
	while(var<10)
	{
	var++;
	printf("%d\n",var);
	}
        pthread_exit(NULL);	
}
void *fun2(void* args)
{
        while(var<20)
	{
        var++;
        printf("%d\n",var);
	}
        pthread_exit(NULL);
}

int main()
{
  pthread_create(&t1,NULL,fun1,NULL);
  pthread_create(&t2,NULL,fun2,NULL);
  pthread_join(t1,NULL);
  pthread_join(t2,NULL);
  pthread_exit(NULL); 
}
