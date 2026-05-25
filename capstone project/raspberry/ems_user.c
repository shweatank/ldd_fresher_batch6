```c id="1g8qfe"
#include <stdio.h>      // Standard input/output functions
#include <fcntl.h>      // File control options
#include <unistd.h>     // POSIX API functions like read(), write(), sleep()
#include <string.h>     // memset(), memcpy()
#include <time.h>       // time() function for timestamps
#include <stdint.h>     // Standard integer types

#include "ems_data_model.h" // Environmental monitoring data structure
#include "uart.h"           // UART communication functions
#include "u_crc16.h"        // CRC16 checksum functions


// Frame start and end markers
#define FRAME_START 0xAA
#define FRAME_END   0x55


/* -------- Structure for Temperature & Humidity -------- */

// Structure used to receive sensor data
struct temp_hum_t
{
	int temp;   // Temperature value
	int hum;    // Humidity value
};


/* -------- Main Function -------- */

int main()
{
    // Initialize UART communication on Raspberry Pi
    int uart_fd = uart_init("/dev/ttyAMA0", 9600);

    // Check whether UART opened successfully
    if(uart_fd < 0)
    {
	    perror("open");
	    return -1;
    }

    // Print UART success message
    printf("ttyAMA0 opened successfully\n with fd : %d\n", uart_fd);


    // Open character device driver for sensor data
    int sensor_fd = open("/dev/thd_driver", O_RDONLY);

    // Check whether driver opened successfully
    if(sensor_fd < 0)
    {
	    perror("open");
	    return -1;
    }

    // Print driver success message
    printf("THD driver opened successfully\n with fd : %d\n",
            sensor_fd);


    // Structure to hold environmental monitoring data
    ems_data_t data;

    // Structure to store temperature & humidity
    struct temp_hum_t th_d;

    // Buffer used for UART data frame
    uint8_t frame[256];


    /* -------- Infinite Monitoring Loop -------- */

    while (1)
    {
        // Clear previous data
        memset(&data, 0, sizeof(data));

        // Read temperature & humidity from driver
        if(read(sensor_fd, &th_d, sizeof(th_d)) < 0)
	     continue;

        // Print sensor readings
	printf("Sensor data Temp = %d, Humidity = %d",
                th_d.temp,
                th_d.hum);

        // Store sensor values into monitoring structure
	data.temp = th_d.temp;
	data.humidity = th_d.hum;

        // Store current timestamp
        time(&data.timestamp);


        /* -------- Frame Preparation -------- */

        // Payload length = size of data structure
        uint16_t payload_len = sizeof(data);

        // Compute CRC16 checksum for payload
        uint16_t crc =
            crc16_compute((uint8_t *)&data, payload_len);

        // Index variable for frame buffer
        int idx = 0;


        /* -------- Add Frame Header -------- */

        // Add frame start byte
        frame[idx++] = FRAME_START;


        /* -------- Add Payload Length -------- */

        // Store lower byte of payload length
        frame[idx++] = payload_len & 0xFF;

        // Store higher byte of payload length
        frame[idx++] = (payload_len >> 8) & 0xFF;


        /* -------- Add Payload Data -------- */

        // Copy sensor data into frame
        memcpy(&frame[idx], &data, payload_len);

        // Move index forward
        idx += payload_len;


        /* -------- Add CRC Checksum -------- */

        // Store lower byte of CRC
        frame[idx++] = crc & 0xFF;

        // Store higher byte of CRC
        frame[idx++] = (crc >> 8) & 0xFF;


        /* -------- Add Frame End Marker -------- */

        // Add frame end byte
        frame[idx++] = FRAME_END;


        /* -------- UART Transmission -------- */

        // Send complete frame through UART
        uart_send(uart_fd, frame, idx);


        // Print transmission message
	printf(" PI SENT FRAME (%d bytes)\n", idx);

        // Debugging: print frame bytes in HEX format
	//for(int i = 0; i < idx; i++)
	//	printf("%02X ",frame[i]);

	printf("\n");


        // Null terminate frame buffer (optional safety)
        frame[idx] = '\0';


        /* -------- Delay -------- */

        // Wait 5 seconds before next transmission
        sleep(5);
    }

    return 0;
}
