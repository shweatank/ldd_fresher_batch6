#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include "uart.h"
#include "crc16.h"
#include "ems_data_model.h"

#define FRAME_START   0xAA
#define FRAME_END     0x55
#define MAX_PAYLOAD   128

// Shared global data and mutex (updated when valid frame is received)
extern ems_data_t g_data;
extern pthread_mutex_t lock;

// Forward declaration for notifying connected clients
void send_to_clients();

/*
 * UART frame parsing states
 * Implements a simple state machine for robust serial frame decoding
 */
typedef enum
{
    WAIT_START,     // Waiting for frame start byte (0xAA)
    READ_LEN,       // Reading 2-byte payload length
    READ_PAYLOAD,   // Reading payload bytes
    READ_CRC,       // Reading CRC16 (2 bytes)
    READ_END        // Verifying frame end byte (0x55)
} state_t;

/*
 * UART receiver thread
 * Continuously reads bytes from UART and reconstructs framed packets
 */
void *uart_thread(void *arg)
{
    (void)arg;

    // Initialize UART device
    int fd = uart_init("/dev/ttyUSB0");

    if (fd < 0)
    {
        perror("uart_init");
        return NULL;
    }

    printf("UART RX thread started\n");

    uint8_t byte;                      // Single incoming byte
    uint8_t payload[MAX_PAYLOAD];     // Buffer for payload data

    uint16_t payload_len = 0;         // Length of payload
    uint16_t crc_recv = 0;            // CRC received from frame
    uint16_t index = 0;               // General-purpose index counter

    state_t state = WAIT_START;       // Initial state of parser

    while (1)
    {
        // Read one byte from UART (blocking read configured in uart_init)
        if (read(fd, &byte, 1) <= 0)
        {
            continue;
        }

        /*
         * Optional debug: print raw incoming bytes
         */
        // printf("%02X ", byte);
        // fflush(stdout);

        switch (state)
        {
            case WAIT_START:
                // Wait for frame start delimiter (0xAA)
                if (byte == FRAME_START)
                {
                    state = READ_LEN;

                    index = 0;
                    payload_len = 0;
                    crc_recv = 0;

                    memset(payload, 0, sizeof(payload));
                }
                break;

            case READ_LEN:
                // Read 2-byte payload length (little-endian format)

                if (index == 0)
                {
                    payload_len = byte;  // LSB
                    index++;
                }
                else
                {
                    payload_len |= ((uint16_t)byte << 8); // MSB

                    index = 0;

                    // Validate payload length
                    if (payload_len == 0 || payload_len > MAX_PAYLOAD)
                    {
                        printf("[UART] Invalid payload length: %u\n", payload_len);

                        state = WAIT_START;
                        payload_len = 0;
                        crc_recv = 0;
                    }
                    else
                    {
                        state = READ_PAYLOAD;
                    }
                }
                break;

            case READ_PAYLOAD:
                // Collect payload bytes

                if (index < payload_len)
                {
                    payload[index++] = byte;
                }
                else
                {
                    // Overflow protection (should not normally happen)
                    printf("[UART] Payload overflow\n");

                    state = WAIT_START;
                    payload_len = 0;
                    crc_recv = 0;
                    index = 0;
                }

                // Move to CRC reading after payload is complete
                if (index >= payload_len)
                {
                    index = 0;
                    state = READ_CRC;
                }
                break;

            case READ_CRC:
                // Read 2-byte CRC (little-endian)

                if (index == 0)
                {
                    crc_recv = byte;  // LSB
                    index++;
                }
                else
                {
                    crc_recv |= ((uint16_t)byte << 8); // MSB

                    index = 0;
                    state = READ_END;
                }
                break;

            case READ_END:
                // Validate frame end byte and verify integrity

                if (byte == FRAME_END)
                {
                    // Compute CRC over received payload
                    uint16_t crc_calc = crc16_compute(payload, payload_len);

                    // Check CRC validity
                    if (crc_calc == crc_recv)
                    {
                        // Ensure payload matches expected struct size
                        if (payload_len == sizeof(ems_data_t))
                        {
                            ems_data_t *rx = (ems_data_t *)payload;

                            // Protect shared data with mutex
                            pthread_mutex_lock(&lock);

                            g_data.temp = rx->temp;
                            g_data.humidity = rx->humidity;
                            g_data.timestamp = rx->timestamp;

                            pthread_mutex_unlock(&lock);

                            // Notify connected clients about new data
                            send_to_clients();
                        }
                        else
                        {
                            printf("[UART] Struct size mismatch\n");
                        }
                    }
                    else
                    {
                        // CRC mismatch debugging output
                        printf("[UART] CRC FAIL recv=0x%04X calc=0x%04X\n",
                               crc_recv,
                               crc_calc);

                        printf("Payload: ");
                        for (int i = 0; i < payload_len; i++)
                        {
                            printf("%02X ", payload[i]);
                        }
                        printf("\n");
                    }
                }
                else
                {
                    // Invalid frame termination byte
                    printf("[UART] Invalid frame end: 0x%02X\n", byte);
                }

                /*
                 * Reset state machine for next frame
                 */
                state = WAIT_START;
                payload_len = 0;
                crc_recv = 0;
                index = 0;

                break;
        }
    }

    return NULL;
}
