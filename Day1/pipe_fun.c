#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2]; // fd[0] is read, fd[1] is write
    pid_t pid;
    int a, b, sum;

    // 1. Create the pipe before forking
    if (pipe(fd) == -1) {
        perror("Pipe failed");
        return 1;
    }

    // 2. Parent takes input
    printf("Enter two numbers: ");
    scanf("%d %d", &a, &b);

    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid == 0) {
        // --- CHILD PROCESS ---
        close(fd[0]); // Close unused read end

        sum = a + b; // Do operation
        
        // Send result to parent through pipe
        write(fd[1], &sum, sizeof(sum));
        close(fd[1]); // Close write end after sending
        exit(0);
    } else {
        // --- PARENT PROCESS ---
        close(fd[1]); // Close unused write end

        wait(NULL); // Wait for child to finish calculation

        // Read the result from the pipe
        read(fd[0], &sum, sizeof(sum));
        printf("Parent received result from child: %d\n", sum);
        
        close(fd[0]); // Close read end
    }

}

