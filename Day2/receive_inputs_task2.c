#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handle_signal(int sig) {
    int a,b ; // = 10, b = 5; // Example numbers for operation
    printf("Enter the inputs a: \n"); scanf("%d",&a);
    printf("Enter the inputs b: \n"); scanf("%d",&b);

    switch (sig) {
        case SIGHUP:  // Signal 1
            printf("SIGHUP (1) received: Addition (10 + 5) = %d\n", a + b);
            break;
        case SIGINT:  // Signal 2
            printf("SIGINT (2) received: Subtraction (10 - 5) = %d\n", a - b);
            break;
        case SIGQUIT: // Signal 3
            printf("SIGQUIT (3) received: Multiplication (10 * 5) = %d\n", a * b);
            break;
        case SIGILL:  // Signal 4
            printf("SIGILL (4) received: Division (10 / 5) = %d\n", a / b);
            break;
        case SIGTRAP:  // Signal 5
             exit(0); break;
        default:
            printf("Received signal %d\n", sig);
    }
}

int main() {
    printf("Receiver PID: %d\n", getpid());
    printf("Waiting for signals (1:Add, 2:Sub, 3:Mul, 4:Div 5: for Quit)...\n");

    // Register handlers for specific signals
    signal(SIGHUP, handle_signal);
    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);
    signal(SIGILL, handle_signal);
    signal(SIGTRAP, handle_signal);

    while (1) {
        pause(); // Wait for the next signal to arrive
    }
    return 0;
}

