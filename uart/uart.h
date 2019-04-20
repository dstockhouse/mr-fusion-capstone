/****************************************************************************\
 *
 * File:
 * 	uart.h
 *
 * Description:
 * 	Function and type declarations and constants for uart.c
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/13/2019
 *
 * Revision 0.2
 * 	Included integration with logger
 * 	Last edited 2/20/2019
 *
 * Revision 0.3
 * 	Split UART and ADS-B specific receiver into separate modules
 * 	Last edited 4/01/2019
 *
 * Revision 0.4
 * 	Added uartWrite function
 * 	Last edited 4/20/2019
 *
\***************************************************************************/

#ifndef __UART_H
#define __UART_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

int UARTInit(char *devName, int baud);

int UARTRead(int uart_fd, char *buf, int length);

int UARTWrite(int uart_fd, char *buf, int length);

int UARTClose(int uart_fd);

#endif

