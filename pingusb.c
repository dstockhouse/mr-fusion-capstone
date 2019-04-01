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
 * 	Last edited 4/01/2019
 *
\***************************************************************************/

#include "pingusb.h"

#include "uart.h"
#include "buffer.h"
#include "logger.h"
#include "crc.h"
#include "ADS_B.h"

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
 * 	dev - Pointer to PINGUSB_RECV instance to initialize
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int pingUSBInit(PINGUSB_RECV *dev) {

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	dev->fd = UARTInit(PINGUSB_RECV_DEV, PINGUSB_RECV_BAUD);
	if(dev->fd < 0) {
		printf("Couldn't initialize pingUSB receiver\n");
		return -1;
	}

	// Initialize the input buffer
	BufferEmpty(&(dev->inbuf));

	// Initialize log file
	LogInit(&(dev->logFile), "SampleData", "ADS_B", 1);

	return 0;

} // pingUSBInit(PINGUSB_RECV *)


/**** Function pingUSBPoll ****
 *
 * Polls a pingUSB UART receiver instance for input
 *
 * Arguments: 
 * 	dev - Pointer to PINGUSB_RECV instance to poll
 *
 * Return value:
 *	Returns number of bytes received (may be 0)
 *	On failure, returns a negative number
 */
int pingUSBPoll(PINGUSB_RECV *dev) {

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

	// Check if UART data avail
	rc = ioctl(dev->fd, FIONREAD, &ioctl_status);
	if(rc) {
		perror("UART: ioctl() failed");
		return -1;
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

} // pingUSBPoll(PINGUSB_RECV *)


/**** Function pingUSBParse ****
 *
 * Parses one packet from the pingUSB's input buffer
 *
 * Arguments: 
 * 	dev  - Pointer to PINGUSB_RECV instance to modify
 * 	data - Pointer to data object to populate with parsed data
 *
 * Return value:
 * 	On success, returns 0, on failure, returns anything else
 */
int pingUSBParse(PINGUSB_RECV *dev) {

	int i, rc, valid = 0, proper = 0, improper = 0;
	uint16_t chkRd, chkNew;

	LOG_FILE packetLogFileRaw, packetLogFileParsed;

	int packfd;
	char parselogextra[LOG_FILENAME_LENGTH+32], parselog[LOG_FILENAME_LENGTH+32];

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	printf("In pingUSBParse, %d bytes in buffer\n", dev->inbuf.length);

	// Seek to next start of packet
	for(i = 0; i < dev->inbuf.length - 46 && (dev->inbuf.buffer[i] != 0xfe || dev->inbuf.buffer[i + 1] != 0x26 || dev->inbuf.buffer[i + 5] != 246); i++) {
		// printf("%d: 0x%x\n", i, dev->inbuf.buffer[i]);
	}

	if(i >= dev->inbuf.length - 46) {
		printf("End of current buffer\n");
		pingUSBConsume(dev, i);
		return 1;
	}

	printf("Parsing index %d (0x%x)\n", i, i);

	chkRd = dev->inbuf.buffer[i + 44] | (dev->inbuf.buffer[i + 45] << 8);
	chkNew = crc_calculate(&(dev->inbuf.buffer[i+1]), 43);
	printf("Read checksum: %04x\n", chkRd);
	printf("Computed chks: %04x\n", chkNew);

	if(chkRd != chkNew) {
		// 			printf("Failed checksum, skipping...\n\n");
		i++;
		improper++;
	}
	printf("Passed checksum, parsing...\n\n");
	proper++;

	// Log the packet at this index to its own file
	// sprintf(parselogextra, ".id%08x.pkt", i);
	// strcpy(parselog, dev->logFile.filename);
	// strcat(parselog, parselogextra);
	// printf("Logging packet to file %s...\n", parselog);
	// packfd = open(parselog, O_WRONLY | O_CREAT);
	// write(packfd, &(dev->inbuf.buffer[i]), 46);
	// close(packfd);

	// Initialize packet log files
	LogInit(&packetLogFileRaw, "SampleData", "ADS_B_packet", 1);
	LogInit(&packetLogFileParsed, "SampleData", "ADS_B_packet", 0);

	rc = parseHeader(&(dev->inbuf.buffer[i]), &(dev->packetHeader), 0);
	// printf("Header:\n");
	// printHeader(&header);

	rc = parseData(&(dev->inbuf.buffer[i + 6]), &(dev->packetData), 0);

	logDataRaw(&packetLogFileRaw, &(dev->inbuf.buffer[i + 6]));
	logDataParsed(&packetLogFileParsed, &(dev->packetData));

	// printf("Data:\n");
	// printData(&data);

	i++;

	pingUSBConsume(dev, i);

	LogClose(&packetLogFileRaw);
	LogClose(&packetLogFileParsed);

	return 0;

} // pingUSBParse(PINGUSB_RECV *)


/**** Function pingUSBConsume ****
 *
 * Consumes bytes in the UART input buffer
 *
 * Arguments: 
 * 	dev - Pointer to PINGUSB_RECV instance to modify
 * 	num - Number of elements to consume
 *
 * Return value:
 * 	On success, returns the number of elements consumed
 *	On failure, returns a negative number 
 */
int pingUSBConsume(PINGUSB_RECV *dev, int num) {

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	return BufferRemove(&(dev->inbuf), num);

} // pingUSBConsume(PINGUSB_RECV *, int)


/**** Function pingUSBDestroy ****
 *
 * De-initializes a pingUSB UART receiver instance
 *
 * Arguments: 
 * 	dev - Pointer to PINGUSB_RECV instance to destroy
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int pingUSBDestroy(PINGUSB_RECV *dev) {

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	// Close UART fd associated with device
	UARTClose(dev->fd);

	// Close log file
	LogClose(&(dev->logFile));

	// Return 0 on success
	return 0;

} // pingUSBDestroy(PINGUSB_RECV *)

