#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "../include/sensor_ioctl.h"
#include "common.h"

static int g_dev_fd = -1;
static int g_epfd = -1;
static int g_event_fd = -1;
static int g_fifo_fd = -1;
static int g_server_fd = -1;
static mqd_t g_alert_mq = (mqd_t)-1;
static struct ring_buffer *g_ring = NULL;

static pthread_t g_sensor_thread;
static pthread_t g_alert_thread;
static pthread_t g_client_thread;
static pthread_t g_logger_thread;

static pthread_mutex_t g_alert_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_alert_cond = PTHREAD_COND_INITIALIZER;
static pthread_rwlock_t g_state_lock = PTHREAD_RWLOCK_INITIALIZER;

static struct sensor_record g_latest;
static atomic_int g_run = 1;
static atomic_int g_alert_pending = 0;
static atomic_int g_emergency_signal = 0;

static void publish_alert(const char *txt, int severity)
{
    struct alert_msg msg;
    snprintf(msg.text, sizeof(msg.text), "%s", txt);
    msg.severity = severity;
    if (g_alert_mq != (mqd_t)-1)
        mq_send(g_alert_mq, (const char *)&msg, sizeof(msg), 0);
}

static void enqueue_ring(struct sensor_record rec)
{
    sem_wait(&g_ring->slots);
    pthread_mutex_lock(&g_ring->lock);
    g_ring->records[g_ring->write_idx] = rec;
    g_ring->write_idx = (g_ring->write_idx + 1) % RING_CAPACITY;
    pthread_mutex_unlock(&g_ring->lock);
    sem_post(&g_ring->items);
}

static int parse_record(const char *line, struct sensor_record *r)
{
    return sscanf(line, "temp=%d vib=%d emergency=%d ts=%lu",
                  &r->temperature, &r->vibration, &r->emergency, &r->ts_ns) == 4;
}

static void *sensor_reader_thread(void *arg)
{
    struct epoll_event ev;
    struct epoll_event out[4];
    char buf[256];

    (void)arg;
    while (atomic_load(&g_run)) {
        int n = epoll_wait(g_epfd, out, 4, 1000);
        for (int i = 0; i < n; i++) {
            if (out[i].data.fd != g_dev_fd)
                continue;

            lseek(g_dev_fd, 0, SEEK_SET);
            ssize_t rd = read(g_dev_fd, buf, sizeof(buf) - 1);
            if (rd <= 0)
                continue;
            buf[rd] = '\0';

            struct sensor_record rec = {0};
            if (!parse_record(buf, &rec))
                continue;

            pthread_rwlock_wrlock(&g_state_lock);
            g_latest = rec;
            pthread_rwlock_unlock(&g_state_lock);

            enqueue_ring(rec);
            ssize_t rc = write(g_event_fd, &(uint64_t){1}, sizeof(uint64_t));
            (void)rc;
        }
    }
    (void)ev;
    return NULL;
}

static void log_to_fifo(const char *line)
{
    ssize_t rc;
    if (g_fifo_fd >= 0) {
        rc = write(g_fifo_fd, line, strlen(line));
        (void)rc;
        rc = write(g_fifo_fd, "\n", 1);
        (void)rc;
    }
}

static void *alert_thread(void *arg)
{
    struct sensor_threshold th = {.temperature_high = 70, .vibration_high = 60};
    char line[160];

    (void)arg;
    while (atomic_load(&g_run)) {
        uint64_t counter = 0;
        if (read(g_event_fd, &counter, sizeof(counter)) <= 0)
            continue;

        pthread_rwlock_rdlock(&g_state_lock);
        struct sensor_record rec = g_latest;
        pthread_rwlock_unlock(&g_state_lock);

        snprintf(line, sizeof(line), "reading temp=%d vib=%d emergency=%d",
                 rec.temperature, rec.vibration, rec.emergency);
        log_to_fifo(line);

        if (rec.temperature > th.temperature_high || rec.vibration > th.vibration_high) {
            int bz = 1;
            ioctl(g_dev_fd, SENSOR_IOCTL_ENABLE_BUZZER, &bz);
            publish_alert("Threshold violation detected", 2);
            log_to_fifo("ALERT threshold violation");
        }

        if (atomic_exchange(&g_emergency_signal, 0)) {
            publish_alert("Emergency interrupt signaled", 3);
            log_to_fifo("CRITICAL emergency signal");
            ssize_t rc = write(g_dev_fd, "emergency_ack", strlen("emergency_ack"));
            (void)rc;
        }

        pthread_mutex_lock(&g_alert_mutex);
        atomic_store(&g_alert_pending, 1);
        pthread_cond_broadcast(&g_alert_cond);
        pthread_mutex_unlock(&g_alert_mutex);
    }
    return NULL;
}

static void *client_thread(void *arg)
{
    int clients[MAX_CLIENTS] = {0};
    (void)arg;

    while (atomic_load(&g_run)) {
        struct sockaddr_un cli_addr;
        socklen_t sz = sizeof(cli_addr);
        int cfd = accept(g_server_fd, (struct sockaddr *)&cli_addr, &sz);
        if (cfd >= 0) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == 0) {
                    clients[i] = cfd;
                    break;
                }
            }
        }

        pthread_mutex_lock(&g_alert_mutex);
        while (!atomic_load(&g_alert_pending) && atomic_load(&g_run))
            pthread_cond_wait(&g_alert_cond, &g_alert_mutex);
        atomic_store(&g_alert_pending, 0);
        pthread_mutex_unlock(&g_alert_mutex);

        pthread_rwlock_rdlock(&g_state_lock);
        struct sensor_record rec = g_latest;
        pthread_rwlock_unlock(&g_state_lock);

        char msg[128];
        int n = snprintf(msg, sizeof(msg), "LIVE temp=%d vib=%d emergency=%d\n",
                         rec.temperature, rec.vibration, rec.emergency);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] > 0 && write(clients[i], msg, n) <= 0) {
                close(clients[i]);
                clients[i] = 0;
            }
        }
    }
    return NULL;
}

static void *logger_notify_thread(void *arg)
{
    (void)arg;
    while (atomic_load(&g_run)) {
        sleep(5);
        log_to_fifo("daemon heartbeat");
    }
    return NULL;
}

static void sigio_handler(int sig)
{
    ssize_t rc;
    (void)sig;
    atomic_store(&g_emergency_signal, 1);
    rc = write(g_event_fd, &(uint64_t){1}, sizeof(uint64_t));
    (void)rc;
}

static int setup_shared_memory(void)
{
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
        return -1;
    if (ftruncate(fd, sizeof(*g_ring)) < 0)
        return -1;

    g_ring = mmap(NULL, sizeof(*g_ring), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (g_ring == MAP_FAILED)
        return -1;

    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&g_ring->lock, &mattr);
    pthread_mutexattr_destroy(&mattr);

    sem_init(&g_ring->slots, 1, RING_CAPACITY);
    sem_init(&g_ring->items, 1, 0);
    g_ring->write_idx = 0;
    g_ring->read_idx = 0;
    return 0;
}

int main(void)
{
    struct mq_attr attr = {.mq_maxmsg = 10, .mq_msgsize = sizeof(struct alert_msg)};
    struct epoll_event ev = {.events = EPOLLIN | EPOLLPRI, .data.fd = -1};

    signal(SIGIO, sigio_handler);

    g_dev_fd = open(SENSOR_DEV_PATH, O_RDWR | O_NONBLOCK);
    if (g_dev_fd < 0) {
        perror("open sensor");
        return 1;
    }

    fcntl(g_dev_fd, F_SETOWN, getpid());
    fcntl(g_dev_fd, F_SETFL, fcntl(g_dev_fd, F_GETFL) | O_ASYNC);
    ioctl(g_dev_fd, SENSOR_IOCTL_START_MONITORING);

    g_epfd = epoll_create1(0);
    g_event_fd = eventfd(0, 0);
    if (g_epfd < 0 || g_event_fd < 0)
        return 1;

    ev.data.fd = g_dev_fd;
    epoll_ctl(g_epfd, EPOLL_CTL_ADD, g_dev_fd, &ev);

    if (mkfifo(FIFO_PATH, 0666) < 0 && errno != EEXIST)
        return 1;
    g_fifo_fd = open(FIFO_PATH, O_WRONLY | O_NONBLOCK);

    if (setup_shared_memory() < 0)
        return 1;

    mq_unlink(ALERT_MQ_NAME);
    g_alert_mq = mq_open(ALERT_MQ_NAME, O_CREAT | O_WRONLY, 0666, &attr);
    if (g_alert_mq == (mqd_t)-1) {
        perror("mq_open create");
        return 1;
    }

    unlink(UNIX_SOCK_PATH);
    g_server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (g_server_fd < 0)
        return 1;
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", UNIX_SOCK_PATH);
    if (bind(g_server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind unix socket");
        return 1;
    }
    if (listen(g_server_fd, 8) < 0) {
        perror("listen unix socket");
        return 1;
    }

    pthread_create(&g_sensor_thread, NULL, sensor_reader_thread, NULL);
    pthread_create(&g_alert_thread, NULL, alert_thread, NULL);
    pthread_create(&g_client_thread, NULL, client_thread, NULL);
    pthread_create(&g_logger_thread, NULL, logger_notify_thread, NULL);

    pthread_join(g_sensor_thread, NULL);
    pthread_join(g_alert_thread, NULL);
    pthread_join(g_client_thread, NULL);
    pthread_join(g_logger_thread, NULL);
    return 0;
}
