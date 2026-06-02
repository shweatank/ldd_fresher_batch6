#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

static struct ring_buffer *g_ring;
static FILE *g_logf;

static void now_string(char *buf, size_t n)
{
    time_t t = time(NULL);
    struct tm tmv;
    localtime_r(&t, &tmv);
    strftime(buf, n, "%F %T", &tmv);
}

static void *shm_consumer(void *arg)
{
    (void)arg;
    while (1) {
        sem_wait(&g_ring->items);
        pthread_mutex_lock(&g_ring->lock);
        struct sensor_record rec = g_ring->records[g_ring->read_idx];
        g_ring->read_idx = (g_ring->read_idx + 1) % RING_CAPACITY;
        pthread_mutex_unlock(&g_ring->lock);
        sem_post(&g_ring->slots);

        char ts[32];
        now_string(ts, sizeof(ts));
        fprintf(g_logf, "[%s] SHM temp=%d vib=%d emergency=%d ts_ns=%lu\n",
                ts, rec.temperature, rec.vibration, rec.emergency, rec.ts_ns);
        fflush(g_logf);
    }
    return NULL;
}

static void *fifo_consumer(void *arg)
{
    int fifo_fd = *(int *)arg;
    char buf[256];
    while (1) {
        ssize_t n = read(fifo_fd, buf, sizeof(buf) - 1);
        if (n <= 0) {
            if (errno == EAGAIN) {
                usleep(100000);
                continue;
            }
            continue;
        }
        buf[n] = '\0';
        char ts[32];
        now_string(ts, sizeof(ts));
        fprintf(g_logf, "[%s] FIFO %s", ts, buf);
        fflush(g_logf);
    }
    return NULL;
}

int main(void)
{
    pthread_t t1, t2;
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    int fifo_fd;

    if (fd < 0) {
        perror("shm_open");
        return 1;
    }
    g_ring = mmap(NULL, sizeof(*g_ring), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (g_ring == MAP_FAILED)
        return 1;

    g_logf = fopen("sensor_events.log", "a");
    if (!g_logf)
        return 1;

    fifo_fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (fifo_fd < 0) {
        perror("open fifo");
        return 1;
    }

    pthread_create(&t1, NULL, shm_consumer, NULL);
    pthread_create(&t2, NULL, fifo_consumer, &fifo_fd);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
