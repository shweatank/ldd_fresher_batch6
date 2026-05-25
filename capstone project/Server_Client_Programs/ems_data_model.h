
#ifndef EMS_DATA_MODEL_H
#define EMS_DATA_MODEL_H

#include <stdint.h>

typedef struct
{
    float min_temp;
    float max_temp;
    float min_humidity;
    float max_humidity;
    char industry[32];
} operating_range_t;

typedef struct __attribute__ ((packed))
{
    float temp;
    float humidity;
    uint64_t timestamp;
} ems_data_t;

#endif
