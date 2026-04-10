/*Simple Producer–Consumer with Signal Stop : Problem: Create a program where -> A parent process creates a child process using IPC (pipe).
      -> The child process uses 2 threads: 1.One thread produces numbers. 2.Another thread sends them to the parent via pipe.
      
      -> The parent reads and prints the numbers.          NOTE: Signal Use: When user presses Ctrl+C (SIGINT), both processes stop gracefully.*/
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
//voltatile sigatomic_t stop =0;
int stop =0;
int buf[100];
int p[2];

int cnt=0;
void handler(int sig){
  stop=1;
}
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
void *producer(void* arg){
     int num = 1;
     while(!stop){
       pthread_mutex_lock(&lock);
       if(cnt<100){
         buf[cnt++]=num;
	 printf("Produed : %d\n",num); 
	 num++;
       }
       pthread_mutex_unlock(&lock);
       sleep(1);
     }
     pthread_exit(NULL);
}
void *consumer(void* arg){
  while(!stop){
    pthread_mutex_lock(&lock);
    
    if(cnt>0){
      int value = buf[--cnt];
      write(p[1],&value,sizeof(value));
    } 
    pthread_mutex_unlock(&lock);
    sleep(1);
  }
  pthread_exit(NULL);
}
int main(){
 signal(SIGINT,handler);	
  pipe(p);
  pid_t pid;
  pid=fork();
  pthread_t th1,th2;
  if(pid==0){
    close(p[0]); //close read end	  
    printf("Child process created \n");
    pthread_create(&th1,NULL,producer,NULL);
    pthread_create(&th2,NULL,consumer,NULL);

    pthread_join(th1,NULL);
    pthread_join(th2,NULL);
    
    close(p[1]); // close write end after writing...
    printf("Child exit gracefully...\n");
  }
  else{
      printf("Parent  process created \n");
      close(p[1]);
      int value;
      while(!stop){
        if(read(p[0],&value,sizeof(value))>0){
	  printf("Received values: %d\n",value);
	}
      }
      close(p[0]);
      wait(NULL);
      printf("Parent exit gracefully...\n");
  }
}
