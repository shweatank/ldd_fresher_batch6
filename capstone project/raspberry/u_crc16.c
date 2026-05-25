
#include "u_crc16.h"   // Header file for CRC16 function definitions


/* ---------------- CRC16 CHECKSUM FUNCTION ---------------- */

/*
 * Compute CRC16 checksum for given data buffer
 * data -> pointer to input data
 * len  -> number of bytes in data buffer
 */
uint16_t crc16_compute(uint8_t *data, size_t len)
{
        // Initialize CRC with standard starting value
        uint16_t crc = 0xFFFF;

        // Process each byte in input data
        for(size_t i = 0; i < len; i++)
        {
                // XOR current byte with CRC register
                crc ^= data[i];

                // Process all 8 bits of current byte
                for(size_t j = 0; j < 8; j++)
                {
                        // Check Least Significant Bit (LSB)
                        if(crc & 1)

                                // Right shift CRC
                                // Apply CRC polynomial 0xA001
                                crc = (crc >> 1) ^ 0xA001;
                        else
                                // If LSB is 0, only shift right
                                crc >>= 1;
                }
        }

        // Return final CRC16 checksum value
        return crc;
}
```

