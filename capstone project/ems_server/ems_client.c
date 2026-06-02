#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>

#include "network_socket.h"
#include "ems_data_model.h"

/*
 * EMS Client Application
 * Continuously receives live sensor data from server
 * and displays it in a formatted terminal view.
 */

int main(int argc, char *argv[])
{
    // Validate command-line arguments (IP and port required)
    if (argc < 3)
    {
        printf("Usage: %s <server-ip> <port>\n", argv[0]);
        return -1;
    }

    // Extract server IP and port from arguments
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    // Establish connection to EMS server
    int fd = connect_peer(ip, port);

    if (fd < 0)
    {
        printf("Connection failed\n");
        return -1;
    }

    printf("[CLIENT] Connected to EMS server\n");

    /*
     * Main receive loop:
     * Continuously reads EMS data packets from server
     */
    while (1)
    {
        ems_data_t data;

        // Receive full EMS data structure from socket
        int n = recv(fd, &data, sizeof(data), 0);

        // If connection is closed or error occurs
        if (n <= 0)
        {
            printf("[CLIENT] Server disconnected\n");
            break;
        }

        // Convert timestamp to human-readable local time
        time_t ts = data.timestamp;
        struct tm *tm_info = localtime(&ts);

        // Display formatted sensor data
        printf("\n=====================================\n");
        printf(" Live Sensor Data\n");
        printf("=====================================\n");

        printf(" Temperature : %.2f C\n", data.temp);
        printf(" Humidity    : %.2f %%\n", data.humidity);

        printf(" Time        : %02d:%02d:%02d\n",
               tm_info->tm_hour,
               tm_info->tm_min,
               tm_info->tm_sec);

        printf("=====================================\n");
    }

    // Close socket connection before exiting
    close(fd);

    return 0;
}
