#ifndef SENSOR_IOCTL_H
#define SENSOR_IOCTL_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define SENSOR_IOC_MAGIC 'q'

struct sensor_threshold {
    __s32 temperature_high;
    __s32 vibration_high;
};

struct sensor_stats {
    __u64 samples;
    __u64 temp_alerts;
    __u64 vib_alerts;
    __u64 emergency_events;
};

#define SENSOR_IOCTL_START_MONITORING _IO(SENSOR_IOC_MAGIC, 1)
#define SENSOR_IOCTL_STOP_MONITORING  _IO(SENSOR_IOC_MAGIC, 2)
#define SENSOR_IOCTL_SET_THRESHOLD    _IOW(SENSOR_IOC_MAGIC, 3, struct sensor_threshold)
#define SENSOR_IOCTL_ENABLE_BUZZER    _IOW(SENSOR_IOC_MAGIC, 4, int)
#define SENSOR_IOCTL_GET_STATS        _IOR(SENSOR_IOC_MAGIC, 5, struct sensor_stats)

#define SENSOR_DEVICE_NAME "sensor_char"
#define SENSOR_CLASS_NAME  "sensor_class"

#endif
