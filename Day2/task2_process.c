#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pid_t pid;
    int a, b, sum;

    if (pipe(fd) == -1) {
        perror("Pipe failed"); return 0;
    }
  while(1){
    printf("Enter two numbers: ");
    scanf("%d %d", &a, &b);

    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 0;
    }

    if (pid == 0) {
        close(fd[0]); 

        sum = a + b; 

        write(fd[1], &sum, sizeof(sum));
        close(fd[1]); 
        exit(0);
    } else {
        close(fd[1]);
        wait(NULL); 
        read(fd[0], &sum, sizeof(sum));
        printf("Parent received result from child: %d\n", sum);
        close(fd[0]);
    }
 }
}

