#ifndef EMS_DATA_MODEL_H
#define EMS_DATA_MODEL_H

#include <stdint.h>

typedef struct __attribute__ ((packed)){
        float temp;
        float humidity;
        uint64_t timestamp;
       // uint16_t crc;
}ems_data_t;

#endif

