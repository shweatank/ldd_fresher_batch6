#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <PID> <SIG_NUM>\n", argv[0]);
        return 1;
    }

    int pid = atoi(argv[1]);
    int sig = atoi(argv[2]);

    // Sends the signal to the specified PID
    if (kill(pid, sig) == -1) {
        perror("Error sending signal");
        return 1;
    }

    printf("Signal %d sent to process %d\n", sig, pid);
    return 0;
}

