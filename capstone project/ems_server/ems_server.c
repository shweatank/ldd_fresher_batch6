#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#include "network_socket.h"
#include "dashboard.h"
#include "ems_data_model.h"
#include "uart_processor.h"

#define SERVER_PORT 8000
#define MAX_CLIENTS 10

/*
 * Global shared EMS data updated by UART thread
 * Protected using mutex for thread safety
 */
ems_data_t g_data;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Client socket table (simple fixed-size connection pool)
 */
static int clients[MAX_CLIENTS];

/*
 * Logging interval (default: 600 seconds = 10 minutes)
 */
static int log_interval_sec = 600;

/*
 * Predefined industry operating ranges
 * Used for threshold comparison / dashboard visualization
 */
static operating_range_t industries[] =
{
    {18, 27, 40, 60, "Data Center"},
    {20, 30, 30, 70, "Pharmaceutical"},
    {15, 35, 20, 80, "Food Industry"},
    {22, 28, 45, 65, "Hospital"}
};

/*
 * Selected industry configuration (modifiable by user)
 */
static operating_range_t selected_range;

/*
 * Broadcast latest EMS data to all connected clients
 */
void send_to_clients()
{
    pthread_mutex_lock(&lock);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] > 0)
        {
            send(clients[i], &g_data, sizeof(g_data), 0);
        }
    }

    pthread_mutex_unlock(&lock);
}

/*
 * User configuration menu for selecting industry profile
 * and optionally customizing operating ranges
 */
static void configure_industry()
{
    int choice;

    printf("\nSelect Industry:\n");
    for (int i = 0; i < 4; i++)
        printf("%d. %s\n", i + 1, industries[i].industry);

    printf("Choice: ");
    scanf("%d", &choice);

    // Default fallback if invalid selection
    if (choice < 1 || choice > 4)
        choice = 1;

    selected_range = industries[choice - 1];

    printf("\nModify default ranges? (1=yes / 0=no): ");
    scanf("%d", &choice);

    if (choice)
    {
        printf("Min Temp: ");
        scanf("%f", &selected_range.min_temp);

        printf("Max Temp: ");
        scanf("%f", &selected_range.max_temp);

        printf("Min Humidity: ");
        scanf("%f", &selected_range.min_humidity);

        printf("Max Humidity: ");
        scanf("%f", &selected_range.max_humidity);
    }
}

/*
 * Server thread:
 * Accepts incoming TCP client connections and stores them
 */
void *server_thread(void *arg)
{
    (void)arg;

    int server_fd = start_server(SERVER_PORT);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);

        // Accept new client connection
        int cfd = accept(server_fd,
                         (struct sockaddr *)&client_addr,
                         &len);

        if (cfd < 0)
            continue;

        // Store client socket in first available slot
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (!clients[i])
            {
                clients[i] = cfd;
                break;
            }
        }
    }

    return NULL;
}

/*
 * UI thread:
 * Continuously refreshes terminal dashboard display
 */
void *ui_thread(void *arg)
{
    (void)arg;

    while (1)
    {
        system("clear");

        int count = 0;
        for (int i = 0; i < MAX_CLIENTS; i++)
            count += clients[i] > 0;

        // Display dashboard with current data and active clients
        start_ui(&g_data, &selected_range, count);

        sleep(1);
    }

    return NULL;
}

/*
 * Utility function to convert timestamp into readable string
 */
static void get_timestamp(char *buffer, size_t size, time_t time)
{
    time_t now = time;
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

/*
 * Logs current sensor data into a file
 */
void log_sensor_data()
{
    FILE *fp = fopen("logs/ems_log.txt", "a");
    if (!fp)
        return;

    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp), g_data.timestamp);

    fprintf(fp,
            "TEMP=%.2fC HUMIDITY=%.2f%% TIMESTAMP=%s\n",
            g_data.temp,
            g_data.humidity,
            timestamp);

    fflush(fp);
    fclose(fp);
}

/*
 * Logger thread:
 * Periodically writes sensor data to log file
 */
void *logger_thread(void *arg)
{
    (void)arg;

    while (1)
    {
        sleep(log_interval_sec);

        pthread_mutex_lock(&lock);
        log_sensor_data();
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

/*
 * Main entry point:
 * Initializes system threads (UART, server, UI, logger)
 */
int main(int argc, char *argv[])
{
    // Optional argument: logging interval in seconds
    if (argc > 1)
        log_interval_sec = atoi(argv[1]);

    // Configure system based on industry profile
    configure_industry();

    pthread_t uart_tid, server_tid, ui_tid, logger_tid;

    // Start system threads
    pthread_create(&uart_tid, NULL, uart_thread, NULL);
    pthread_create(&server_tid, NULL, server_thread, NULL);
    pthread_create(&ui_tid, NULL, ui_thread, NULL);
    pthread_create(&logger_tid, NULL, logger_thread, NULL);

    // Keep main thread alive by waiting on UART thread
    pthread_join(uart_tid, NULL);

    return 0;
}
