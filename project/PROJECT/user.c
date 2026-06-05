#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>

/* Path of IR/UART kernel driver device file */
#define IR_DRIVER_PATH "/dev/ir_sensor_dev"

/* Path of OLED kernel driver device file */
#define OLED_DRIVER_PATH "/dev/oled"

/* IOCTL command to send our PID to IR driver */
#define IOCTL_SET_VALUE _IOW('a', 'a', int)

/* Global OLED file descriptor so signal handlers can use it */
int oled_fd;

// Handler for '1' (Permission Granted)
void handle_sigusr1(int sig) {
    /* Message to show on OLED when permission is granted */
    const char *msg = "PERMISSION GRANTED";

    printf("[SIGNAL] Received '1' from UART. Updating OLED...\n");

    /* If OLED device is open, write message to OLED driver */
    if (oled_fd > 0) {
        write(oled_fd, msg, strlen(msg));
    }
}

// Handler for '0' (Permission Denied)
void handle_sigusr2(int sig) {
    /* Message to show on OLED when permission is denied */
    const char *msg = "PERMISSION DENIED";

    printf("[SIGNAL] Received '0' from UART. Updating OLED...\n");

    /* If OLED device is open, write message to OLED driver */
    if (oled_fd > 0) {
        write(oled_fd, msg, strlen(msg));
    }
}

int main() {
    int ir_fd;              /* File descriptor for IR/UART driver */
    int pid = getpid();     /* Get current process ID */

    // Open both drivers character device files(nodes)
    ir_fd = open(IR_DRIVER_PATH, O_RDWR);
    oled_fd = open(OLED_DRIVER_PATH, O_RDWR);

    /* Check if both drivers opened successfully */
    if (ir_fd < 0 || oled_fd < 0)
    {
        perror("Failed to open one or both drivers");
        return -1;
    }

    // Register Signal Handlers
    signal(SIGINT, SIG_IGN); /* Ignore Ctrl+C so app keeps running */

    /* When driver sends SIGUSR1, call handle_sigusr1 */
    signal(SIGUSR1, handle_sigusr1);

    /* When driver sends SIGUSR2, call handle_sigusr2 */
    signal(SIGUSR2, handle_sigusr2);

    // Register this App's PID with the IR/UART Driver
    /* Driver will use this PID to send signals back to this app */
    if (ioctl(ir_fd, IOCTL_SET_VALUE, &pid) < 0)
    {
        perror("IOCTL Registration Failed");
        return -1;
    }

    printf("Bridge App Running. PID: %d. Waiting for UART data...\n", pid);

    /* Wait forever until driver sends SIGUSR1 or SIGUSR2 */
    while (1)
    {
        pause(); // Sleep until a signal arrives from the IR/UART kernel module
    }

    /* Close device files before exit (not reached in normal flow) */
    close(ir_fd);
    close(oled_fd);
    return 0;
}
