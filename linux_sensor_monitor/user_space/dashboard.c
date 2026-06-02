#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "common.h"

static int g_sock = -1;
static mqd_t g_mq = (mqd_t)-1;

static void *live_view_thread(void *arg)
{
    char buf[256];
    (void)arg;
    while (1) {
        ssize_t n = read(g_sock, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("[LIVE] %s", buf);
        }
    }
    return NULL;
}

static void *alert_view_thread(void *arg)
{
    struct alert_msg msg;
    (void)arg;
    while (1) {
        if (mq_receive(g_mq, (char *)&msg, sizeof(msg), NULL) > 0)
            printf("[ALERT][%d] %s\n", msg.severity, msg.text);
    }
    return NULL;
}

int main(void)
{
    pthread_t t1, t2;
    struct sockaddr_un addr = {.sun_family = AF_UNIX};

    g_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (g_sock < 0)
        return 1;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", UNIX_SOCK_PATH);
    if (connect(g_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return 1;
    }

    g_mq = mq_open(ALERT_MQ_NAME, O_RDONLY);
    if (g_mq == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    pthread_create(&t1, NULL, live_view_thread, NULL);
    pthread_create(&t2, NULL, alert_view_thread, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
