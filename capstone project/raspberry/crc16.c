#include "crc16.h"

// Function to compute CRC16 checksum
// data -> pointer to input data array
// len  -> number of bytes in the data array
u16 crc16_compute(u8 *data, size_t len)
{
        // Initialize CRC with starting value 0xFFFF
        // Common initial value for Modbus CRC16
        u16 crc = 0xFFFF;

        // Loop through each byte of input data
        for(size_t i = 0; i < len; i++)
        {
                // XOR current CRC value with current data byte
                crc ^= data[i];

                // Process all 8 bits of the current byte
                for(size_t j = 0; j < 8; j++)
                {
                        // Check if Least Significant Bit (LSB) is 1
                        if(crc & 1)

                                // Right shift CRC by 1 bit
                                // Then apply CRC polynomial 0xA001
                                crc = ( crc >> 1 ) ^ 0xA001;
                        else
                                // If LSB is 0, only right shift CRC
                                crc >>= 1;
                }
        }

        // Return final computed CRC16 checksum
        return crc;
}
