/****************************************************************************\
 *
 * File:
 * 	VN200_GPS.c
 *
 * Description:
 * 	Interfaces with an GPS reciever connected to serial port through USB
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 4/16/2019
 *
 * Revision 0.2
 * 	Last edited 5/06/2019
 *
\***************************************************************************/

#include "VN200_GPS.h"

#include "../uart/uart.h"
#include "../buffer/buffer.h"
#include "../logger/logger.h"
#include "crc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>


/**** Function VN200GPSInit ****
 *
 * Initializes a pingUSB UART receiver instance
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to initialize
 * 	fs  - Sampling frequency to initialize the module to
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int VN200GPSInit(VN200_DEV *dev, int fs) {

#define CMD_BUFFER_SIZE 64
	char commandBuf[CMD_BUFFER_SIZE];
	int commandBufLen;

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	// Initialize UART for use
	VN200BaseInit(dev);

	// Disable asynchronous output
	commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,6,0");
	VN200Command(dev, commandBuf, commandBufLen);

	// Set sampling frequency
	dev->fs = fs;
	snprintf(commandBuf, CMD_BUFFER_SIZE, "%s%d", "VNWRG,7,", dev->fs);
	VN200Command(dev, commandBuf, commandBufLen);

	// Clear input buffer (temporary)
	usleep(100000);
	VN200FlushInput(dev);
	usleep(100000);

	// Enable asynchronous GPS data output
	snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,6,20");
	VN200Command(dev, commandBuf, commandBufLen);

	// Clear input buffer (temporary)
	VN200FlushInput(dev);

	return 0;

} // VN200GPSInit(VN200_DEV *, int)


/**** Function VN200GPSParse ****
 *
 * Initializes a pingUSB UART receiver instance
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to initialize
 *
 * Return value:
 *	On success, returns number of bytes parsed from buffer
 *	On failure, returns a negative number
 */
int VN200GPSParse(VN200_DEV *dev, GPS_DATA *parsedData) {

	// Make extra sure there is enough room in the buffer
#define PACKET_BUF_SIZE 1024
	char currentPacket[PACKET_BUF_SIZE];

#define NUM_GPS_FIELDS 15
	char *tokenList[NUM_GPS_FIELDS];

	unsigned char chkOld, chkNew;

	int packetStart, packetEnd, i;

	// Exit on error if invalid pointer
	if(dev == NULL || parsedData == NULL) {
		return -1;
	}

	// Initialize token list to null
	for(i = 0; i < NUM_GPS_FIELDS; i++) {
		tokenList[i] = NULL;
	}

	packetStart = 0;
	// while(!valid) {

	// Find start of a packet ($)
	for( ; packetStart < dev->inbuf.length && 
			dev->inbuf.buffer[packetStart] != '$'; packetStart++) ;

	// Find end of packet (*)
	for(packetEnd = packetStart; packetEnd < dev->inbuf.length - 3 && 
		dev->inbuf.buffer[packetEnd] != '*'; packetEnd++) ;

	if(packetStart >= dev->inbuf.length - 3 || packetEnd >= dev->inbuf.length - 3) {
		return 0;
	}

	if(packetEnd - packetStart > PACKET_BUF_SIZE - 1) {
		return -3;
	}

	// Verify checksum
	// sscanf(dev->inbuf.buffer[packetEnd + 1], "%x", &chkOld);
	// chkNew = calculateChecksum(dev->inbuf.buffer[packetStart], packetEnd - packetStart);

	// Copy string to be modified by strtok
	strncpy(currentPacket, &(dev->inbuf.buffer[packetStart]), packetEnd-packetStart);

	// Parse between commas (not exactly safe)
	tokenList[0] = strtok(currentPacket, ",");
	for(i = 1; i < NUM_GPS_FIELDS && tokenList[i-1] != NULL; i++) {
		tokenList[i] = strtok(NULL, ",");
	}
	printf("Read %d GPS comma delimited fields from %d characters\n", i-1, packetEnd);

	// Get time
	sscanf(tokenList[0], "%lf", &(parsedData->time));

	// Get number of GPS satellites
	sscanf(tokenList[3], "%hhd", &(parsedData->NumSats));

	// Get latitude
	sscanf(tokenList[4], "%lf", &(parsedData->Latitude));

	// Get Longitude
	sscanf(tokenList[5], "%lf", &(parsedData->Longitude));

	// Get Altitude
	sscanf(tokenList[6], "%lf", &(parsedData->Altitude));

	return packetEnd;

} // VN200GPSParse(VN200_DEV *, GPS_DATA *)

