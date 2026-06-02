#include "crc16.h"

// CRC16 implementation using binary polynomial division (bitwise XOR + shifts)
// This version uses the CRC-16/MODBUS polynomial (0xA001, reverse form of 0x8005)

uint16_t crc16_compute(uint8_t *data, size_t len)
{
    // Initialize CRC register with all bits set (standard starting value for many CRC16 variants)
    uint16_t crc = 0xFFFF; // 16-bit CRC accumulator

    // Process each byte in the input data buffer
    for(size_t i = 0; i < len; i++)
    {
        // XOR current byte into the lower 8 bits of the CRC register
        crc ^= data[i];

        // Process each bit in the current byte (8 bits)
        for(size_t j = 0; j < 8; j++)
        {
            // Check if the least significant bit (LSB) is 1
            if(crc & 1)
            {
                // If LSB is 1:
                // Shift right and apply polynomial XOR correction
                crc = (crc >> 1) ^ 0xA001; // Polynomial used in CRC-16/MODBUS
            }
            else
            {
                // If LSB is 0:
                // Just shift right without XOR correction
                crc >>= 1;
            }
        }
    }

    // Final CRC value after processing all bytes
    // This is the remainder of polynomial division
    return crc;
}
