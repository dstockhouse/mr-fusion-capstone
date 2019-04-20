/***************************************************************************\
 *
 * File:
 * 	vn200.c
 *
 * Description:
 *	Basic common functionality for VN200, GPS and IMU
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 4/20/2019
 *
 ***************************************************************************/

#include "vn200.h"

#include "../uart/uart.h"
#include "../buffer/buffer.h"
#include "../logger/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>


/**** Function vn200BaseInit ****
 *
 * Initializes a VN200 IMU/GPS before it is setup for either functionality
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to initialize
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int vn200BaseInit(VN200_DEV *dev) {

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1; }

	dev->fd = UARTInit(VN200_DEVNAME, VN200_BAUD);
	if(dev->fd < 0) {
		printf("Couldn't initialize VN200 sensor\n");
		return -2;
	}

	// Initialize the input and output buffers
	BufferEmpty(&(dev->inbuf));
	BufferEmpty(&(dev->outbuf));

	// Initialize log file
	LogInit(&(dev->logFile), "../SampleData/VN200", "ADS_B", 1);

	return 0;

} // vn200BaseInit(VN200_DEV *)


/**** Function vn200Poll ****
 *
 * Polls the UART file for an initialized VN200 device and populates inbuf with
 * read data
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to poll
 *
 * Return value:
 *	On success, returns the number of bytes received (may be 0)
 *	On failure, returns a negative number
 */
int vn200Poll(VN200_DEV *dev) {

	int numToRead, numRead, rc, ioctl_status;
	char *startBuf, tempBuf[BYTE_BUFFER_LEN];

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

	// Check if UART data available
	rc = ioctl(dev->fd, FIONREAD, &ioctl_status);
	if(rc) {
		perror("UART: ioctl() failed");
		return -3;
	}
	// printf("%d bytes avail...\n", ioctl_status);

	// Calculate length and pointer to proper position in array
	numToRead = BYTE_BUFFER_LEN - dev->inbuf.length;
	startBuf = &(dev->inbuf.buffer[dev->inbuf.length]);
	// printf("Poll: startBuf is %p\n", startBuf);
	startBuf[0] = 2;

	// printf("Attempting to read %d bytes from uart device...\n", numToRead);

	// Read without blocking from pingUSB UART device
	numRead = UARTRead(dev->fd, startBuf, numToRead);
	// numRead = UARTRead(dev->fd, tempBuf, numToRead);
	// printf("\tRead %d\n", numRead);

	// Log newly read data to file
	LogUpdate(&(dev->logFile), startBuf, numRead);
	// LogUpdate(&(dev->logFile), tempBuf, numRead);

	// memcpy(&(dev->inbuf.buffer[dev->inbuf.length]), tempBuf, numRead);

	dev->inbuf.length += numRead;

	// Return number successfully read (may be 0)
	return numRead;

} // vn200Poll(VN200_DEV *)


/**** Function vn200Consume ****
 *
 * Consumes bytes in the UART input buffer
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to modify
 * 	num - Number of bytes to consume
 *
 * Return value:
 *	On success, returns number of bytes consumed
 *	On failure, returns a negative number
 */
int vn200Consume(VN200_DEV *dev, int num) {

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	return BufferRemove(&(dev->inbuf), num);

} // vn200Consume(VN200_DEV *, int)


/**** Function vn200Write ****
 *
 * Writes data to the VN200 output buffer, then flushes the output to UART
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to modify
 * 	buf - Pointer to data to be written out
 * 	num - Number of bytes to write
 *
 * Return value:
 *	On success, returns number of bytes written
 *	On failure, returns a negative number
 */
int vn200Write(VN200_DEV *dev, char *buf, int num) {

	/* For item in buf, copy to dev->outbuf
	 * 	dev->outbuf[item] = buf[item]
	 *
	 * compute CRC, put at end of outbuf
	 * 	dev->outbuf[last] = "*" & crc

}

int vn200Flush(VN200_DEV *dev);

int vn200Destroy(VN200_DEV *dev);

