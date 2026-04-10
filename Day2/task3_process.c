#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int p_to_c[2], c_to_p[2]; 
    pid_t pid;

    if (pipe(p_to_c) == -1 || pipe(c_to_p) == -1) {
        perror("Pipe failed");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1; 
    }

    if (pid > 0) { 
	// Parent Process
        int input_val, result;
        close(p_to_c[0]);
        close(c_to_p[1]); 

        printf("Parent: Enter a number to square: ");
        scanf("%d", &input_val);

        write(p_to_c[1], &input_val, sizeof(input_val)); 
        read(c_to_p[0], &result, sizeof(result));    

        printf("Parent: Received result from child: %d\n", result);

        close(p_to_c[1]);
        close(c_to_p[0]);
        wait(NULL); 
    } 
    else { // Child Process
        int received_val, squared_val;
        close(p_to_c[1]); 
        close(c_to_p[0]); 

        read(p_to_c[0], &received_val, sizeof(received_val)); 
        squared_val = received_val * received_val;          
        write(c_to_p[1], &squared_val, sizeof(squared_val)); 

        close(p_to_c[0]);
        close(c_to_p[1]);
    }
}

