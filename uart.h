/****************************************************************************\
 *
 * File:
 * 	uart.h
 *
 * Description:
 * 	Header files and function headers for the UART interface
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/13/2019
 *
\***************************************************************************/

#ifndef __UART_H
#define __UART_H

#include "buffer.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#define USB_RECV_DEV "/dev/ttyUSB0"
// #define USB_RECV_DEV "/dev/ttyACM0"
#define USB_RECV_BAUD 115200

typedef struct {
	int fd;
	BYTE_BUFFER inbuf;
	LOG_FILE logFile;
} USB_RECV;

int UARTInit(char *devName, int baud);

int UARTRead(int uart_fd, char *buf, int length);

int UARTClose(int uart_fd);

int pingUSBInit(USB_RECV *dev);

int pingUSBPoll(USB_RECV *dev);

int pingUSBDestroy(USB_RECV *dev);

#endif

