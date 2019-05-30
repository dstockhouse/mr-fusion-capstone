/****************************************************************************\
 *
 * File:
 * 	pingusb.c
 *
 * Description:
 * 	Interfaces with an ADS-B reciever connected to serial port through USB
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Split UART and ADB functionality
 * 	Last edited 4/01/2019
 *
 * Revision 0.2
 * 	Renamed structure type to PINGUSB_DEV
 * 	Last edited 4/20/2019
 *
\***************************************************************************/

#include "pingusb.h"

#include "../uart/uart.h"
#include "../buffer/buffer.h"
#include "../logger/logger.h"
#include "crc.h"
#include "adsb_parser.h"

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


/**** Function pingUSBInit ****
 *
 * Initializes a pingUSB UART receiver instance
 *
 * Arguments: 
 * 	dev - Pointer to PINGUSB_DEV instance to initialize
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int pingUSBInit(PINGUSB_DEV *dev) {

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1; }

	dev->fd = UARTInit(PINGUSB_DEVNAME, PINGUSB_BAUD);
	if(dev->fd < 0) {
		printf("Couldn't initialize pingUSB receiver\n");
		return -2;
	}

	// Initialize the input buffer
	BufferEmpty(&(dev->inbuf));

	// Initialize log file
	LogInit(&(dev->logFile), "../SampleData/ADS_B", "ADS_B", LOG_FILEEXT_BIN);
	LogInit(&(dev->logFileParsed), "../SampleData/ADS_B", "ADS_B", LOG_FILEEXT_LOG);

	return 0;

} // pingUSBInit(PINGUSB_DEV *)


/**** Function pingUSBPoll ****
 *
 * Polls a pingUSB UART receiver instance for input
 *
 * Arguments: 
 * 	dev - Pointer to PINGUSB_DEV instance to poll
 *
 * Return value:
 *	Returns number of bytes received (may be 0)
 *	On failure, returns a negative number
 */
int pingUSBPoll(PINGUSB_DEV *dev) {

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
		perror("pingUSBPoll: ioctl() failed");
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

} // pingUSBPoll(PINGUSB_DEV *)


/**** Function pingUSBParse ****
 *
 * Parses one packet from the pingUSB's input buffer
 *
 * Arguments: 
 * 	dev  - Pointer to PINGUSB_DEV instance to modify
 * 	data - Pointer to data object to populate with parsed data
 *
 * Return value:
 * 	If a packet was parsed, returns 1
 * 	If the end of the buffer was reached, return 0
 * 	On error, returns a negative number
 */
int pingUSBParse(PINGUSB_DEV *dev) {

	int i, rc, valid = 0;
	uint16_t chkRd, chkNew;

	// LOG_FILE packetLogFileRaw, packetLogFileParsed;

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	i = 0;
	while(!valid) {
		// printf("In pingUSBParse, %d bytes in buffer\n", dev->inbuf.length);

		// Seek to next start of packet
		for( ; i < dev->inbuf.length - 46 && (dev->inbuf.buffer[i] != 0xfe || dev->inbuf.buffer[i + 1] != 0x26 || dev->inbuf.buffer[i + 5] != 246); i++) {
			// printf("%d: 0x%x\n", i, dev->inbuf.buffer[i]);
		}

		// printf("Attempt stop at index %d\n", i);

		if(i >= dev->inbuf.length - 46) {
			printf("End of current buffer\n");
			pingUSBConsume(dev, i);
			return 0;
		}

		// printf("Parsing index %d (0x%x)\n", i, i);

		chkRd = dev->inbuf.buffer[i + 44] | (dev->inbuf.buffer[i + 45] << 8);
		chkNew = crc_calculate(&(dev->inbuf.buffer[i+1]), 43);
		// printf("Read checksum: %04x\n", chkRd);
		// printf("Computed chks: %04x\n", chkNew);

		if(chkRd == chkNew) {
			valid = 1;

			// printf("Passed checksum, parsing...\n\n");

			// Initialize packet log files
			// LogInit(&packetLogFileRaw, "../SampleData/ADS_B", "ADS_B_packet", 1);
			// LogInit(&packetLogFileParsed, "../SampleData/ADS_B", "ADS_B_packet", 0);

			rc = parseHeader(&(dev->inbuf.buffer[i]), &(dev->packetHeader), 0);

			rc = parseData(&(dev->inbuf.buffer[i + 6]), &(dev->packetData), 0);

			// logDataRaw(&packetLogFileRaw, &(dev->inbuf.buffer[i + 6]));
			// logDataParsed(&packetLogFileParsed, &(dev->packetData));
			logDataParsed(&(dev->logFileParsed), &(dev->packetData));
		}

		i++;

		// printf("Resetting loop at index %d\n", i);

	} // while(!valid)

	pingUSBConsume(dev, i);

	// LogClose(&packetLogFileRaw);
	// LogClose(&packetLogFileParsed);

	return 1;

} // pingUSBParse(PINGUSB_DEV *)


/**** Function pingUSBConsume ****
 *
 * Consumes bytes in the UART input buffer
 *
 * Arguments: 
 * 	dev - Pointer to PINGUSB_DEV instance to modify
 * 	num - Number of bytes to consume
 *
 * Return value:
 * 	On success, returns the number of bytes consumed
 *	On failure, returns a negative number 
 */
int pingUSBConsume(PINGUSB_DEV *dev, int num) {

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	return BufferRemove(&(dev->inbuf), num);

} // pingUSBConsume(PINGUSB_DEV *, int)


/**** Function pingUSBDestroy ****
 *
 * De-initializes a pingUSB UART receiver instance
 *
 * Arguments: 
 * 	dev - Pointer to PINGUSB_DEV instance to destroy
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int pingUSBDestroy(PINGUSB_DEV *dev) {

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	// Close UART fd associated with device
	UARTClose(dev->fd);

	// Close log file
	LogClose(&(dev->logFile));
	LogClose(&(dev->logFileParsed));

	// Return 0 on success
	return 0;

} // pingUSBDestroy(PINGUSB_DEV *)

