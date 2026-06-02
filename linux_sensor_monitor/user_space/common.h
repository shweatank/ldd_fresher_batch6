#ifndef USER_COMMON_H
#define USER_COMMON_H

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

#define SENSOR_DEV_PATH "/dev/sensor_char"
#define SHM_NAME "/sensor_ring_shm"
#define ALERT_MQ_NAME "/sensor_alert_mq"
#define FIFO_PATH "/tmp/sensor_log_fifo"
#define UNIX_SOCK_PATH "/tmp/sensor_dashboard.sock"

#define RING_CAPACITY 128
#define MAX_CLIENTS 8

struct sensor_record {
    int temperature;
    int vibration;
    int emergency;
    uint64_t ts_ns;
};

struct ring_buffer {
    pthread_mutex_t lock;
    sem_t slots;
    sem_t items;
    uint32_t write_idx;
    uint32_t read_idx;
    struct sensor_record records[RING_CAPACITY];
};

struct alert_msg {
    int severity;
    char text[128];
};

#endif
