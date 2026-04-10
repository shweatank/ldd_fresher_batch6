#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msg_buf {
    long msg_type;
    int value;
} message;

int main() {
    key_t key = ftok("progfile", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    printf("Receiver: Waiting for data...\n");

    msgrcv(msgid, &message, sizeof(message.value), 1, 0);
    
    int squared = message.value * message.value;
    printf("Receiver: Processed %d to %d\n", message.value, squared);

    message.msg_type = 2;
    message.value = squared;
    msgsnd(msgid, &message, sizeof(message.value), 0);
}

