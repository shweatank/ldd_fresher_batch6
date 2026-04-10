#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

struct msg_buffer {
    long msg_type;
    int data[2];
};

int main() {
    key_t key = ftok("progfile", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    struct msg_buffer msg;

    if (fork() == 0) { // Child Process
        msgrcv(msgid, &msg, sizeof(msg.data), 1, 0);
        msg.data[0] = msg.data[0] + msg.data[1];
        msg.msg_type = 2;
        msgsnd(msgid, &msg, sizeof(int), 0);
        exit(0);
    } else { // Parent Process
        printf("Enter two numbers: ");
        scanf("%d %d", &msg.data[0], &msg.data[1]);
        msg.msg_type = 1;
        msgsnd(msgid, &msg, sizeof(msg.data), 0);

        msgrcv(msgid, &msg, sizeof(int), 2, 0);
        printf("Parent: The sum is %d\n", msg.data[0]);
        wait(NULL);
        msgctl(msgid, IPC_RMID, NULL);
    }
    return 0;
}

