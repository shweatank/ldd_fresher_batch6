#ifndef UART_H
#define UART_H

int uart_init(const char *device);
int uart_send(int fd, void *data, int len);
int uart_recv(int fd, void *buf, int len);

#endif
