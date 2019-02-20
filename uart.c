/****************************************************************************\
 *
 * File:
 * 	uart.c
 *
 * Description:
 * 	Interfaces with the ADS-B reciever connected to serial port through USB
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/13/2019
 *
\***************************************************************************/

#include "uart.h"

#include "buffer.h"
#include "logger.h"

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
 * Opens and initializes a UART device. Based in part on Derek Molloy's RPi book
 * and wiringPi serial library source
 *
 * Arguments: 
 * 	devName - String name of the file the UART device is at
 * 	baud    - Requested baud rate (for now must be 115200)
 *
 * Return value:
 * 	On success, returns file descriptor corresponding to UART device
 *	On failure, prints error message and returns a negative number 
 */
int UARTInit(char *devName, int baud) {

	struct termios uartOptions;
	int uart_fd, ioctl_status, rc;

	// Exit on error if invalid pointer
	if(devName == NULL) {
		return -1;
	}

	uart_fd = open(devName, O_RDWR | O_NOCTTY | O_NDELAY);
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

	close(uart_fd);

	return 0;

} // UARTClose(int)


/**** Function pingUSBInit ****
 *
 * Initializes a pingUSB UART receiver instance
 *
 * Arguments: 
 * 	dev - Pointer to USB_RECV instance to initialize
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int pingUSBInit(USB_RECV *dev) {

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	// Initialize the input buffer
	BufferEmpty(&(dev->inbuf));

	// Initialize log file
	LogInit(&(dev->logFile), "SampleData", "ADS_B", 1);

	dev->fd = UARTInit(USB_RECV_DEV, USB_RECV_BAUD);
	if(dev->fd < 0) {
		printf("Couldn't initialize pingUSB receiver\n");
		return -1;
	}

	return 0;

} // pingUSBInit(USB_RECV *)


/**** Function pingUSBPoll ****
 *
 * Polls a pingUSB UART receiver instance for input
 *
 * Arguments: 
 * 	dev - Pointer to USB_RECV instance to poll
 *
 * Return value:
 *	Returns number of bytes received (may be 0)
 *	On failure, returns a negative number
 */
int pingUSBPoll(USB_RECV *dev) {

	int numToRead, numRead, rc, ioctl_status;
	unsigned char *startBuf;

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	// Ensure length of buffer is long enough to hold more data
	if(dev->inbuf.length >= BYTE_BUFFER_LEN) {
		printf("Can't poll pingUSB receiver: input buffer is full (%d bytes)",
				dev->inbuf.length);
		return -2;
	}

	// Check if UART data avail
	rc = ioctl (dev->fd, FIONREAD, &ioctl_status);
	if(rc) {
		perror("UART: ioctl() failed");
		return -1;
	}
	// printf("%d bytes avail...\n", ioctl_status);

	// Calculate length and pointer to proper position in array
	numToRead = BYTE_BUFFER_LEN - dev->inbuf.length;
	startBuf = &(dev->inbuf.buffer[dev->inbuf.length]);

	// Read without blocking from pingUSB UART device
	numRead = UARTRead(dev->fd, startBuf, numToRead);

	dev->inbuf.length += numRead;

	// Return number successfully read (may be 0)
	return numRead;

} // pingUSBPoll(USB_RECV *)


/**** Function pingUSBDestroy ****
 *
 * De-initializes a pingUSB UART receiver instance
 *
 * Arguments: 
 * 	dev - Pointer to USB_RECV instance to destroy
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int pingUSBDestroy(USB_RECV *dev) {

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	// Close log file
	LogClose(&(dev->logFile));

	// Close UART fd associated with device
	UARTClose(dev->fd);

	// Return 0 on success
	return 0;

} // pingUSBDestroy(USB_RECV *)

