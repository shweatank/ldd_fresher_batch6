#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "uart.h"

// Initialize UART device and configure serial communication parameters
int uart_init(const char *device)
{
    // Set device file permissions to allow read/write access
    if(chmod(device, 0666) == -1)
    {
        perror("chmod failed");
        // exit(1); // intentionally disabled for robustness
    }

    // Open UART device file (non-controlling terminal mode)
    int fd = open(device, O_RDWR | O_NOCTTY);
    if(fd < 0)
    {
        perror("open");
        // exit(1); // intentionally disabled for robustness
    }

    struct termios opt; // Structure containing UART/TTY configuration settings
    memset(&opt, 0, sizeof(opt));

    // Retrieve current terminal attributes into opt
    if(tcgetattr(fd, &opt) != 0)
    {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    // Set input and output baud rate to 9600 bps
    cfsetispeed(&opt, B9600); // RX (receive) speed
    cfsetospeed(&opt, B9600); // TX (transmit) speed

    // Enable receiver and ignore modem control lines (important for UART communication)
    opt.c_cflag |= (CLOCAL | CREAD);

    // Configure frame format: 8 data bits, no parity, 1 stop bit (8N1)
    opt.c_cflag &= ~PARENB;   // Disable parity bit
    opt.c_cflag &= ~CSTOPB;   // Use 1 stop bit instead of 2
    opt.c_cflag &= ~CSIZE;    // Clear current data size setting
    opt.c_cflag |= CS8;       // Set 8-bit character size

    // Disable canonical mode and special processing
    // Canonical mode waits for newline; disabling allows raw byte-by-byte input
    opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // Disable software flow control and special input processing
    // IXON/IXOFF disable software flow control
    // ICRNL/INLCR/IGNCR prevent newline/carriage return conversions
    opt.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL | INLCR | IGNCR);

    // Disable output processing (raw output mode)
    opt.c_oflag &= ~OPOST;

    // Read configuration:
    // VMIN = 1 → block until at least 1 byte is received
    // VTIME = 0 → no timeout
    opt.c_cc[VMIN] = 1;
    opt.c_cc[VTIME] = 0;

    // Flush any existing input data in buffer
    tcflush(fd, TCIFLUSH);

    // Apply the configuration immediately
    if(tcsetattr(fd, TCSANOW, &opt) != 0)
    {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    printf("Uart initialized successfully\n");
    return fd;
}

// Send data over UART
int uart_send(int fd, void *data, int len)
{
    return write(fd, data, len);
}

// Receive data from UART
int uart_recv(int fd, void *buf, int len)
{
    return read(fd, buf, len);
}
