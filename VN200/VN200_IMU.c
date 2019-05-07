/****************************************************************************\
 *
 * File: 
 * 	VN200_IMU.c
 *
 * Description: 
 * 	Initialize the VN200 to gather and send IMU data
 *
 * Author:
 * 	Joseph Kroeker
 *
 * Revision 0.1
 * 	Last edited 4/1/2019
 *
\****************************************************************************/

#include "VN200.h"
#include "VN200_IMU.h"

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

/**** Function VN200IMUInit ****
 *
 * Initialize the VN200 for IMU data
 *
 * Arguments:
 * 	dev - Pointer to VN200_DEV instance to initialize
 * 	fs  - Sampling frequency to initialize the module to
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int VN200IMUInit(VN200_DEV *dev, int fs) {

#define CMD_BUFFER_SIZE 64
	char commandBuf[CMD_BUFFER_SIZE];
	int commandBufLen;

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	// Initialize UART for use
	VN200BaseInit(dev);

	// Initialize log file
	LogInit(&(dev->logFile), "../SampleData/VN200/IMU", "VN200", 1);

	// Request IMU serial number
	commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNRRG,03");
	VN200Command(dev, commandBuf, commandBufLen);
	usleep(100000);
	VN200FlushInput(dev);

	// Disable asynchronous data output
	commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG, 6,0");
	VN200Command(dev, commandBuf, commandBufLen);
	
	// Set the asynchronous data output freq
	commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "VNWRG, 7, %d", dev->fs);
	VN200Command(dev, commandBuf, commandBufLen);
	usleep(100000);
	VN200FlushInput(dev);

	// Enable async IMU Measurements on VN200
	commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG, 6, 19");
	VN200Command(dev, commandBuf, commandBufLen);
	usleep(100000);
	VN200FlushInput(dev);

	return 0;

} // VN200IMUInit(VN200_DEV *, int)


/**** Function VN200IMUParse ****
 *
 * Parses data from VN200 input buffer, assuming device is configured as IMU
 *
 * Arguments: 
 * 	dev  - Pointer to VN200_DEV instance to parse from
 * 	data - Pointer to IMU_DATA instance to store parsed data
 *
 * Return value:
 *	On success, returns number of bytes parsed from buffer
 *	On failure, returns a negative number
 */
int VN200IMUParse(VN200_DEV *dev, IMU_DATA *data) {

	unsigned char chkOld, chkNew;
	int packetStart, packetEnd, i;

	// Exit on error if invalid pointer
	if(dev == NULL || data == NULL) {
		return -1;
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

	// Verify checksum
	// sscanf(dev->inbuf.buffer[packetEnd + 1], "%x", &chkOld);
	// chkNew = calculateChecksum(dev->inbuf.buffer[packetStart], packetEnd - packetStart);


	// Parse out values (all doubles)
	sscanf(dev->inbuf.buffer, "$VNIMU,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
			data->compass[0], data->compass[1], data->compass[2],
			data->accel[0], data->accel[1], data->accel[2],
			data->gyro[0], data->gyro[1], data->gyro[2],
			data->temp, data->baro);
	return 0;

} // VN200IMUParse(VN200_DEV *, IMU_DATA *)

