/*Number Sender with Pause/Resume
Problem:
A child process generates numbers using a thread.
Numbers are sent to parent using a pipe.
Parent prints the numbers.
Signal Use:
SIGUSR1 → Pause sending
SIGUSR2 → Resume sending*/
#include<signal.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<pthread.h>
pthread_mutex_t lock;
int p[2];
int cnt=0,stop=0;
int pas = 0;

void stophandler(int sig){
  stop=1;
}
void pausehandler(int sig){
  pas=1;
}
void resumehandler(int sig){
  pas=0;
}
void *generator(void* arg){
  int val=1;	
  while(!stop){ //0 !0 = 1
        if(!pas){ 
	  write(p[1],&val,sizeof(val));
	  val++;
	  usleep(500000);
	}
	else{
	  usleep(500000);
	}
  }
  pthread_exit(NULL); // major mistake pthread lock unlock must be in while loop;
}
int main(){
  signal(SIGUSR1,pausehandler);
  signal(SIGUSR2,resumehandler);
  signal(SIGINT,stophandler);
  if(pipe(p)==-1){
    perror("Pipe failed"); return 1;
  }
  pid_t pid=fork();
  if(pid==0){
     close(p[0]);
     
     pthread_t th1;
     pthread_create(&th1,NULL,generator,NULL);
     pthread_join(th1,NULL);
     close(p[1]);
     printf("child exit gracefully...\n");
  }
  else{
     close(p[1]);	  
     int val;	 
        printf("Parent PID: %d\n", getpid());
        printf("Use:\n");
        printf("kill -SIGUSR1 %d  (Pause)\n", pid);
        printf("kill -SIGUSR2 %d  (Resume)\n", pid);

     while(!stop){ 
     if(read(p[0],&val,sizeof(val))>0){
        printf("Rec: %d\n",val);     
     }
    }
     close(p[0]);
     wait(NULL); //mistake
     printf("Parent exit gracefully...\n");
   
  }
}
