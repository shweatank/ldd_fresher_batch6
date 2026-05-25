#ifndef CRC16_H
#define CRC16_H

#include <linux/types.h>

u16 crc16_compute(u8 *data, size_t len);

#endif

