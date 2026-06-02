#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../include/sensor_ioctl.h"
#include "common.h"

int main(void)
{
    int fd = open(SENSOR_DEV_PATH, O_RDWR);
    mqd_t mq = mq_open(ALERT_MQ_NAME, O_WRONLY);
    FILE *f = fopen("critical_events.log", "a");
    struct sensor_stats prev = {0}, now = {0};

    if (fd < 0 || mq == (mqd_t)-1 || !f)
        return 1;

    while (1) {
        if (ioctl(fd, SENSOR_IOCTL_GET_STATS, &now) == 0) {
            if (now.temp_alerts > prev.temp_alerts || now.vib_alerts > prev.vib_alerts) {
                int bz = 1;
                struct alert_msg msg = {.severity = 2};
                snprintf(msg.text, sizeof(msg.text), "Alert manager: threshold exceeded");
                ioctl(fd, SENSOR_IOCTL_ENABLE_BUZZER, &bz);
                mq_send(mq, (const char *)&msg, sizeof(msg), 0);
                fprintf(f, "critical threshold event: temp_alerts=%llu vib_alerts=%llu\n",
                        (unsigned long long)now.temp_alerts,
                        (unsigned long long)now.vib_alerts);
                fflush(f);
            }
            prev = now;
        }
        sleep(2);
    }
    return 0;
}
