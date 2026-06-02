CC = gcc
CFLAGS = -Wall -pthread -g

all: ems_server ems_client

ems_server: ems_server.o uart.o uart_processor.o network_socket.o crc16.o dashboard.o
$(CC) $(CFLAGS) ems_server.o uart.o uart_processor.o network_socket.o crc16.o dashboard.o -o ems_server

ems_client: ems_client.o network_socket.o
$(CC) $(CFLAGS) ems_client.o network_socket.o -o ems_client

ems_server.o: ems_server.c
$(CC) $(CFLAGS) -c ems_server.c

uart.o: uart.c
$(CC) $(CFLAGS) -c uart.c

uart_processor.o: uart_processor.c
$(CC) $(CFLAGS) -c uart_processor.c

network_socket.o: network_socket.c
$(CC) $(CFLAGS) -c network_socket.c

crc16.o: crc16.c
$(CC) $(CFLAGS) -c crc16.c

dashboard.o: dashboard.c
$(CC) $(CFLAGS) -c dashboard.c

ems_client.o: ems_client.c
$(CC) $(CFLAGS) -c ems_client.c

clean:
rm -f *.o ems_server ems_client
