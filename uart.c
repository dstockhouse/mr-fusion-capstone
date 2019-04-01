/****************************************************************************\
 *
 * File:
 * 	uart.c
 *
 * Description:
 * 	Interfaces with a USB serial device
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
\***************************************************************************/

#include "uart.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>


/**** Function UARTInit ****
 *
 * Opens and initializes a UART device. Based mostly on Derek Molloy's RPi book
 *
 * Arguments: 
 * 	devName - String name of the file the UART device is at
 * 	baud    - Requested baud rate (for now must be either 115200 or 57600)
 *
 * Return value:
 * 	On success, returns file descriptor corresponding to UART device
 *	On failure, prints error message and returns a negative number 
 */
int UARTInit(char *devName, int baud) {

	struct termios uartOptions;
	int uart_fd, rc;

	// Exit on error if invalid pointer
	if(devName == NULL) {
		return -1;
	}

	// uart_fd = open(devName, O_RDWR | O_NOCTTY | O_NDELAY);
	uart_fd = open(devName, O_RDWR | O_NOCTTY);
	if(uart_fd < 0) {
		perror("open() failed for UART device");
		return uart_fd;
	}

	rc = tcgetattr(uart_fd, &uartOptions);
	if(rc) {
		perror("tcgetattr() failed for UART device");
		return -4;
	}

	// Set baud rate based on input, bare minimum so far supported
	switch(baud) {
		case 115200:
			uartOptions.c_cflag |= B115200;
			break;
		case 57600:
			uartOptions.c_cflag |= B57600;
			break;
		default:
			printf("Unexpected baud rate: %d\n", baud);
			close(uart_fd);
			return -3;
			break;
	}

	// Set other termios control options
	uartOptions.c_cflag |= CS8 | CREAD | CLOCAL;

	// Set input options
	uartOptions.c_iflag |= IGNPAR;

	// Push changed options to device (after flushing input and output)
	rc = tcflush(uart_fd, TCIOFLUSH);
 	if(rc) {
 		perror("tcflush() failed for UART device");
 		return -4;
 	}

	rc = tcsetattr(uart_fd, TCSANOW, &uartOptions);
	if(rc) {
		perror("tcsetattr() failed for UART device");
		return -4;
	}

	// Return file descriptor for UART
	return uart_fd;

} // UARTInit(char *, int)


/**** Function UARTRead ****
 *
 * Reads from an open and initialized file descriptor. Based on Derek Molloy's
 * RPi book
 *
 * Arguments: 
 * 	uart_fd - File descriptor for open and initialized UART device
 *	buf     - Buffer to store data that is read
 * 	length  - Length of room left in the buffer
 *
 * Return value:
 *	Returns number of characters read (may be 0)
 *	On failure, prints error message and returns a negative number 
 */
int UARTRead(int uart_fd, char *buf, int length) {

	int numRead;

	// Exit on error if invalid pointer
	if(buf == NULL) {
		return -1;
	}

	// Ensure non-blocking read
	// if(!(fcntl(uart_fd, F_GETFL) & O_NONBLOCK)) {
	// 	fcntl(uart_fd, F_SETFL, O_NONBLOCK);
	// }

	// Attempt to read from UART device at most length bytes
	numRead = read(uart_fd, buf, length);
	// printf("UARTRead: read %d chars\n", numRead);
	if(numRead < 0 && errno != EWOULDBLOCK) {
		perror("read() failed for UART device");
		return -2;
	}

	// Return number of bytes successfully read into buffer
	return numRead;

} // UARTRead(int, char *, int)


/**** Function UARTClose ****
 *
 * Closes the file descriptor for a UART device
 *
 * Arguments: 
 * 	uart_fd - File descriptor for the device to close
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int UARTClose(int uart_fd) {

	int rc;

	rc = close(uart_fd);
	if(rc) {
		perror("Couldn't close UART file");
		return -1;
	}

	return 0;

} // UARTClose(int)

