#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid1, pid2;

    // --- Create First Child Process ---
    pid1 = fork();
    if (pid1 < 0) {
        perror("First fork failed");
        exit(1);
    } else if (pid1 == 0) {
        // Child 1: Run 'ls -l'
        printf("Child 1 (PID: %d) executing 'ls'\n", getpid());
        execlp("ls", "ls", "-l", NULL);
        
        // exec only returns if it fails
        perror("execlp failed");
        exit(1);
    }

    // --- Create Second Child Process ---
    // We only call fork() again from the parent to avoid 4 processes
    pid2 = fork();
    if (pid2 < 0) {
        perror("Second fork failed");
        exit(1);
    } else if (pid2 == 0) {
        // Child 2: Run 'date'
        printf("Child 2 (PID: %d) executing 'date'\n", getpid());
        execlp("date", "date", NULL);
        
        perror("execlp failed");
        exit(1);
    }

    // --- Parent Process ---
    printf("Parent (PID: %d) waiting for children...\n", getpid());
    
    // Wait for both children to finish to avoid zombie processes
    wait(NULL); 
    wait(NULL);

    printf("Parent: Both children finished.\n");
}

