#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>

// Structure to hold ADC readings and calculated values
    int ldr;                 // Raw ADC values

// Global file descriptors for cleanup in ISR
int fd;
FILE *fp;

// Interrupt Service Routine to handle Ctrl+C
void my_isr(int sig) {
    close(fd);      // Close serial port
    fclose(fp);     // Close log file
    exit(0);        // Exit program
}

int main() {
    // Open serial port for reading
    fd = open("/dev/ttyUSB0", O_RDONLY | O_NOCTTY);
    if (fd < 0) {
        perror("Open failed");
        return 1;
    }

    // Open log file
    fp = fopen("logs.txt", "w");
    if (!fp) {
        perror("fopen");
	return 1;
    }

    // Attach signal handler for Ctrl+C
    signal(SIGINT, my_isr);

    // Configure serial port
    struct termios tty;
    tcgetattr(fd, &tty);

    // Set baud rate to 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // 8N1 configuration (8 data bits, No parity, 1 stop bit)
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore modem control lines
    tty.c_cflag &= ~CRTSCTS;       // Disable hardware flow control

    // Raw input/output mode
    tty.c_lflag = 0;
    tty.c_iflag = 0;
    tty.c_oflag = 0;

    tcsetattr(fd, TCSANOW, &tty);

    // Variables for reading data and logging
    time_t t;
    char *timee;
    char buf[100], logs[200];

    // Main loop to read from serial and log values
    while (1) {
	    int n = read(fd, buf, sizeof(buf) - 1);
	    if (n > 0) {
		    buf[n] = '\0'; // Null-terminate buffer

		    // Parse ADC values from buffer
		    sscanf(buf, "%d", &ldr);

		    int ldr_percentage=(ldr*100)/4095;
		    // Get current time
		    t = time(NULL);
		    timee = ctime(&t) + 11;         // Skip day and month
		    timee[strlen(timee) - 6] = '\0'; // Remove newline and year

		    // Prepare log string
int log_len;
if(ldr<100){
            log_len = snprintf(logs, sizeof(logs) - 1,
                "[%s] LDR ADC value %d\t\tLDR Percentage %d%%\t\tLED=ON \n",
                timee, ldr, ldr_percentage);
}
else{
            log_len = snprintf(logs, sizeof(logs) - 1,
                "[%s] LDR ADC value %d\t\tLDR Percentage %d%%\t\tLED=OFF \n",
                timee, ldr, ldr_percentage);
}

            logs[log_len] = '\0';

            // Print and save to file
            printf("%s", logs);
            fprintf(fp, "%s", logs);
        }
    }

    // Cleanup (will never reach here due to infinite loop)
    close(fd);
    fclose(fp);

    return 0;
}

