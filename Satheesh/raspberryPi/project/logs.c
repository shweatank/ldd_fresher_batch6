/*==============================================================*/
/* UART LOG RECEIVER                                            */
/* Reads logs from /dev/ttyUSB0 and stores into log file        */
/* Appends logs with timestamp                                  */
/*==============================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>

/*==============================================================*/

#define UART_PORT   "/dev/ttyUSB0"
#define LOG_FILE    "uart_logs.txt"

/*==============================================================*/

int main(void)
{
    int uart_fd,file_fd;

    struct termios options;

    char buffer[256];

    int n;

    time_t now;

    struct tm *t;

    char time_buf[64];

    /*----------------------------------------------------------*/
    /* Open UART device                                         */
    /*----------------------------------------------------------*/

    uart_fd = open(UART_PORT,O_RDWR | O_NOCTTY);

    if(uart_fd < 0) {

        perror("UART OPEN FAILED");

        return -1;
    }

    /*----------------------------------------------------------*/
    /* UART Configuration                                       */
    /*----------------------------------------------------------*/

    tcgetattr(uart_fd,&options);

    cfsetispeed(&options,B115200);
    cfsetospeed(&options,B115200);

    options.c_cflag |= (CLOCAL | CREAD);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;

    options.c_cflag |= CS8;

    options.c_lflag = 0;
    options.c_iflag = 0;
    options.c_oflag = 0;

    tcsetattr(uart_fd,TCSANOW,&options);

    /*----------------------------------------------------------*/
    /* Open log file in append mode                             */
    /*----------------------------------------------------------*/

    file_fd = open(LOG_FILE,
                   O_WRONLY | O_CREAT | O_APPEND,
                   0644);

    if(file_fd < 0) {

        perror("FILE OPEN FAILED");

        close(uart_fd);

        return -1;
    }

    printf("Listening UART Logs From %s\n", UART_PORT);

    /*----------------------------------------------------------*/
    /* Infinite loop                                            */
    /*----------------------------------------------------------*/

    while(1) {

        memset(buffer,0,sizeof(buffer));

        n = read(uart_fd,buffer,sizeof(buffer)-1);

        if(n > 0) {

            /*--------------------------------------------------*/
            /* Get current time                                 */
            /*--------------------------------------------------*/

            now = time(NULL);

            t = localtime(&now);

            strftime(time_buf,
                     sizeof(time_buf),
                     "[%Y-%m-%d %H:%M:%S]",
                     t);

            /*--------------------------------------------------*/
            /* Print on terminal                                */
            /*--------------------------------------------------*/

            printf("%s %s", time_buf, buffer);

            /*--------------------------------------------------*/
            /* Store into file                                  */
            /*--------------------------------------------------*/

            dprintf(file_fd,
                    "%s %s",
                    time_buf,
                    buffer);

            fsync(file_fd);
        }
    }

    /*----------------------------------------------------------*/

    close(file_fd);

    close(uart_fd);

    return 0;
}
