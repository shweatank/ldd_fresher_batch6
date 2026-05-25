```c id="2k7mqp"
#include <fcntl.h>      // File control options
#include <unistd.h>     // POSIX API functions like read(), write(), close()
#include <stdio.h>      // Standard input/output functions
#include <string.h>     // memset()
#include <sys/stat.h>   // chmod() function
#include <termios.h>    // UART terminal control structure
#include <stdlib.h>     // exit()

#include "uart.h"       // UART function declarations


/* ---------------- UART INITIALIZATION ---------------- */

/*
 * Initialize UART communication
 * device -> UART device file (example: /dev/ttyAMA0)
 * baud   -> communication baud rate
 */
int uart_init(const char *device, int baud)
{
        // Change UART device permissions
        if(chmod(device, 0666) == -1)
        {
                perror("chmod failed");
                exit(1);
        }

        // Open UART device in read/write mode
        int fd = open(device, O_RDWR | O_NOCTTY);

        // Check whether UART opened successfully
        if(fd < 0)
        {
             perror("open");
             exit(1);
        }

        // UART configuration structure
        struct termios opt;

        // Clear structure before configuration
        memset(&opt, 0, sizeof(opt));

        // Get current UART settings
        if(tcgetattr(fd, &opt) != 0)
        {
                perror("tcgetattr");

                close(fd);

                return -1;
        }


        /* -------- Set Baud Rate -------- */

        // Set UART input baud rate
        cfsetispeed(&opt, B9600);

        // Set UART output baud rate
        cfsetospeed(&opt, B9600);


        /* -------- UART Control Flags -------- */

        // Enable receiver and local mode
        opt.c_cflag |= (CLOCAL | CREAD);

        // Disable parity checking
        opt.c_cflag &= ~PARENB;

        // Use 1 stop bit
        opt.c_cflag &= ~CSTOPB;

        // Clear current character size mask
        opt.c_cflag &= ~CSIZE;

        // Set 8 data bits
        opt.c_cflag |= CS8;


        /* -------- UART Local Flags -------- */

        // Disable canonical mode and echo
        opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);


        /* -------- UART Input Flags -------- */

        // Disable software flow control
        // Disable carriage return/newline translations
        opt.c_iflag &= ~(IXON  |
                         IXOFF |
                         IXANY |
                         ICRNL |
                         INLCR |
                         IGNCR);


        /* -------- UART Output Flags -------- */

        // Disable output processing
        opt.c_oflag &= ~OPOST;


        /* -------- Read Configuration -------- */

        // Minimum number of characters to read
        opt.c_cc[VMIN] = 1;

        // Read timeout disabled
        opt.c_cc[VTIME] = 0;


        /* -------- Apply UART Settings -------- */

        // Flush UART input buffer
        tcflush(fd, TCIFLUSH);

        // Apply new UART configuration immediately
        if(tcsetattr(fd, TCSANOW, &opt) != 0)
        {
                perror("tcsetattr");

                close(fd);

                return -1;
        }

        // Print success message
        printf("Uart initialized successfully\n");

        // Return UART file descriptor
        return fd;
}


/* ---------------- UART SEND ---------------- */

/*
 * Send data through UART
 * fd   -> UART file descriptor
 * data -> pointer to transmit buffer
 * len  -> number of bytes to send
 */
int uart_send(int fd, void *data, int len)
{
        // Write data to UART
        return write(fd, data, len);
}


/* ---------------- UART RECEIVE ---------------- */

/*
 * Receive data from UART
 * fd  -> UART file descriptor
 * buf -> buffer to store received data
 * len -> number of bytes to read
 */
int uart_recv(int fd, void *buf, int len)
{
        // Read data from UART
        return read(fd, buf, len);
}
