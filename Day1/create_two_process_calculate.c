#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <num1> <num2>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        printf("Child (PID %d) adding %s + %s...\n", getpid(), argv[1], argv[2]);
        
        execlp("expr", "expr", argv[1], "+", argv[2], (char *)NULL);
        
        perror("Exec failed");
        exit(1);
    } else {
        wait(NULL);
        printf("Parent (PID %d): Child finished the operation.\n", getpid());
    }
}

