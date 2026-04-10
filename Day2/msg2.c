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
    
    printf("Sender: Enter a number: ");
    scanf("%d", &message.value);

    message.msg_type = 1;
    msgsnd(msgid, &message, sizeof(message.value), 0);

    msgrcv(msgid, &message, sizeof(message.value), 2, 0);
    printf("Sender: Received result: %d\n", message.value);

    msgctl(msgid, IPC_RMID, NULL);
}

