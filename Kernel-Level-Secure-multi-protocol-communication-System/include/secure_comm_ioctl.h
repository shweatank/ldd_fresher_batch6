/* Header file for secure communication ioctl definitions. This file defines the ioctl commands and associated constants for controlling the secure communication mode of a UART device. It includes definitions for setting and getting the secure mode (encrypt, decrypt, pass) using ioctl system calls. The ioctl commands are defined using the _IOW and _IOR macros, which specify the command type, command number, and data type for the ioctl operations. */
#ifndef SECURE_COMM_IOCTL_H
#define SECURE_COMM_IOCTL_H

#include <linux/ioctl.h>

#define SECURE_UART_IOC_MAGIC 'U'

#define SECURE_MODE_ENCRYPT 1
#define SECURE_MODE_DECRYPT 2
#define SECURE_MODE_PASS    3

#define SECURE_UART_IOC_SET_MODE _IOW(SECURE_UART_IOC_MAGIC, 1, int)
#define SECURE_UART_IOC_GET_MODE _IOR(SECURE_UART_IOC_MAGIC, 2, int)

#endif
