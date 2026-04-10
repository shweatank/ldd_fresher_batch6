#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

// Define the message structure
struct msg_buffer {
    long msg_type;     // Must be > 0
    int result;        // The actual data we want to send
};

int main() {
    int msgid;
    key_t key;
    struct msg_buffer message;
    int a, b;

    // 1. Generate a unique key and create the message queue
    key = ftok("progfile", 65); 
    msgid = msgget(key, 0666 | IPC_CREAT);

    printf("Parent: Enter two numbers to add: ");
    scanf("%d %d", &a, &b);

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }

    if (pid == 0) {
        // --- CHILD PROCESS ---
        message.msg_type = 1;
        message.result = a + b; // Perform operation

        // Send the result to the queue
        msgsnd(msgid, &message, sizeof(message.result), 0);
        printf("Child: Sent sum %d to Message Queue.\n", message.result);
        exit(0);
    } else {
        // --- PARENT PROCESS ---
        wait(NULL); // Wait for child to finish calculation

        // Receive the message from the queue (msg_type 1)
        msgrcv(msgid, &message, sizeof(message.result), 1, 0);

        printf("Parent: Received result from Child via Message Queue: %d\n", message.result);

        // 2. Destroy the message queue after use
        msgctl(msgid, IPC_RMID, NULL);
    }

    return 0;
}
/*How to Run:

    Compile: gcc msg_ipc.c -o msg_ipc
    Execute: ./msg_ipc
    
*/
