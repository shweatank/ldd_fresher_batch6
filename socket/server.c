#include <stdio.h>      
#include <string.h>     
#include <unistd.h>     
#include <fcntl.h>      
#include <termios.h>    // UART configuration functions
#include <arpa/inet.h>  // Socket programming functions

#define PORT 5002       // TCP server port number

int main()
{
	// File descriptor for UART device
	int uart_fd;

	// File descriptor for server socket
	int server_fd;

	// File descriptor for connected client socket
	int client_fd;

	// Structures used to store server and client address information
	struct sockaddr_in server, client;

	// Variable used to store client address length
	socklen_t len;

	// Buffer to hold data received from UART
	char buffer[1024];

	// Structure used for UART settings
	struct termios tty;

	/* Open UART device */
	uart_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);

	/* Check whether UART opened successfully */
	if (uart_fd < 0) {
		perror("UART open failed");
		return -1;
	}

	/* Get current UART configuration */
	tcgetattr(uart_fd, &tty);

	/* Set UART baud rate to 115200 */
	cfsetispeed(&tty, B115200);
	cfsetospeed(&tty, B115200);

	/* Configure UART:
	   - 8 data bits
	   - Enable receiver
	   - Ignore modem control lines
	 */
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_cflag |= (CLOCAL | CREAD);

	/* Disable parity bit */
	tty.c_cflag &= ~PARENB;

	/* Use one stop bit */
	tty.c_cflag &= ~CSTOPB;

	/* Configure UART in raw mode */
	tty.c_lflag = 0;

	/* Disable software flow control */
	tty.c_iflag = 0;

	/* Disable output processing */
	tty.c_oflag = 0;

	/* Apply UART settings immediately */
	tcsetattr(uart_fd, TCSANOW, &tty);

	printf("UART ready (Minicom active)\n");

	/* Create a TCP socket */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	/* Set server address details */
	server.sin_family = AF_INET;         // IPv4
	server.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP
	server.sin_port = htons(PORT);       // Server port number

	/* Bind socket to the specified port */
	bind(server_fd, (struct sockaddr *)&server, sizeof(server));

	/* Start listening for client connections */
	listen(server_fd, 5);

	printf("Waiting for client...\n");

	/* Wait for a client to connect */
	len = sizeof(client);
	client_fd = accept(server_fd, (struct sockaddr *)&client, &len);

	// Optional second client connection
	// int client_fd_1 = accept(server_fd, (struct sockaddr *)&client, &len);

	printf("Client connected\n");

	/* Main loop */
	while (1)
	{
		/* Read data from UART */
		int len = read(uart_fd, buffer, sizeof(buffer));

		/* If data is received */
		if (len > 0)
		{
			/* Send UART data to connected TCP client */
			write(client_fd, buffer, len);

			// Send data to second client if required
			// write(client_fd_1, buffer, len);

			/* Print transmitted data on terminal */
			printf("Sent: %.*s\n", len, buffer);
		}
	}

	/* Close UART device */
	close(uart_fd);

	/* Close client socket */
	close(client_fd);

	/* Close server socket */
	close(server_fd);

	return 0;
}

