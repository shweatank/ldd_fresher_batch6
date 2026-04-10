/*Multi-Threaded File Reader with Signal Exit
Problem:
A child process reads a file using multiple threads (each thread reads part of file).
Sends data to parent via pipe.
Parent prints the content.
Signal Use:
SIGTERM → Stop reading and exit cleanly*/
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<signal.h>
#define TH 3
#define SIZE 100
volatile sig_atomic_t stop = 0;
FILE *fp;
int p[2];
void handler(int sig){
  stop = 1;
}
void* reader_thread(void* arg) {
    char buffer[SIZE];

    while (!stop) {
        // Critical section: reading file
        if (fgets(buffer,SIZE, fp) == NULL) {
            break; // EOF
        }

        write(p[1], buffer, strlen(buffer));
        usleep(200000);
    }

    pthread_exit(NULL);
}

int main(){
  signal(SIGTERM,handler);
  if(pipe(p)==-1){
    perror("pipe failed"); exit(1);
  }
  pid_t pid=fork();
  if(pid < 0){ perror("fork"); exit(1);}
  if(pid==0){
      close(p[0]); //read end closed

      fp=fopen("a.txt","r");
      if(!fp){perror("file open failed"); exit(1);}

      pthread_t tid[3];
      for (int i = 0; i < 3; i++) {
            pthread_create(&tid[i], NULL, reader_thread, NULL);
        }

        for (int i = 0; i < 3; i++) {
            pthread_join(tid[i], NULL);
        }

        fclose(fp);
        close(p[1]);

        printf("Child exiting cleanly...\n");
  }
  else{
     close(p[1]);
     char buf[100];
       printf("Parent PID: %d\n", getpid());
        printf("Send SIGTERM using: kill -SIGTERM %d\n\n", getpid());
     while(!stop){
       int n=read(p[0],buf,SIZE-1);
       if(n>0){
         buf[n]='\0';
	 printf("Parent Rec: %s\n",buf);
       }
     }
     close(p[0]);
     wait(NULL);
     printf("Parent exiting cleanly...\n");
  }
}
