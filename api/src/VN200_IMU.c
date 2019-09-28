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

#include "uart.h"
#include "buffer.h"
#include "logger.h"
#include "VN200_CRC.h"

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
	char commandBuf[CMD_BUFFER_SIZE], logBuf[256];
	int commandBufLen, logBufLen;

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	// Initialize UART for use
	VN200BaseInit(dev);

	// Initialize log file for raw and parsed data
	LogInit(&(dev->logFile), "../SampleData/VN200/IMU", "VN200", LOG_FILEEXT_LOG);
	LogInit(&(dev->logFileIMUParsed), "../SampleData/VN200/IMU", "VN200", LOG_FILEEXT_CSV);

	// Write header to CSV data
	logBufLen = snprintf(logBuf, 256, "compx,compy,compz,accelx,accely,accelz,gyrox,gyroy,gyroz,temp,baro,timestamp\n");
	LogUpdate(&(dev->logFileIMUParsed), logBuf, logBufLen);

	// Request IMU serial number
	commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNRRG,03");
	VN200Command(dev, commandBuf, commandBufLen, 1);
	usleep(100000);

	// Disable asynchronous data output
	commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,06,0");
	VN200Command(dev, commandBuf, commandBufLen, 1);
	usleep(100000);
	
	// Set the asynchronous data output freq
	dev->fs = fs;
	commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "VNWRG,07,%d", dev->fs);
	VN200Command(dev, commandBuf, commandBufLen, 1);
	usleep(100000);

	// Enable async IMU Measurements on VN200
	commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,06,19");
	VN200Command(dev, commandBuf, commandBufLen, 1);
	usleep(100000);

	// Clear input buffer (temporary)
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
	int packetStart, packetEnd, logBufLen, i, rc;
	char logBuf[512];
	struct timespec timestamp_ts;

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

	// printf("Packet (start, end): %d %d\n", packetStart, packetEnd);
	// printf("                     %c %c\n", dev->inbuf.buffer[packetStart], dev->inbuf.buffer[packetEnd]);

	// Verify checksum
	// printf("Reading checksum\n");
	sscanf(&(dev->inbuf.buffer[packetEnd + 1]), "%hhX", &chkOld);
	chkNew = VN200CalculateChecksum(&(dev->inbuf.buffer[packetStart + 1]), packetEnd - packetStart - 1);
	// printf("Checksum (read, computed): %02X, %02X\n", chkOld, chkNew);

	if(chkNew != chkOld) {
		// Checksum failed, don't parse (but allow to skip to next packet)
		return 1;
	}

	// Make timestamp
	rc = clock_gettime(CLOCK_REALTIME, &timestamp_ts);
	if(rc) {
		perror("VN200IMUParse: Couldn't get timestamp");

		// Set timestamp to 0
		timestamp_ts.tv_sec = 0;
		timestamp_ts.tv_nsec = 0;
	}
	data->timestamp = ((double) timestamp_ts.tv_sec) + ((double) timestamp_ts.tv_nsec) / 1000000000;

	/*
	printf("\n\nData should be \n");
	for(i = packetStart; i < packetEnd + 3; i++) {
		printf("%c", dev->inbuf.buffer[i]);
	}
	printf("\n\n");
	*/

	// Parse out values (all doubles)
	sscanf(&(dev->inbuf.buffer[packetStart]), "$VNIMU,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
			&(data->compass[0]), &(data->compass[1]), &(data->compass[2]),
			&(data->accel[0]), &(data->accel[1]), &(data->accel[2]),
			&(data->gyro[0]), &(data->gyro[1]), &(data->gyro[2]),
			&(data->temp), &(data->baro));

	// Log parsed data to file in CSV format
	logBufLen = snprintf(logBuf, 512, "%.4lf,%.4lf,%.4lf,%.3lf,%.3lf,%.3lf,%.6lf,%.6lf,%.6lf,%.1lf,%.3lf,%.9lf\n",
			data->compass[0], data->compass[1], data->compass[2],
			data->accel[0], data->accel[1], data->accel[2],
			data->gyro[0], data->gyro[1], data->gyro[2],
			data->temp, data->baro, data->timestamp);

	LogUpdate(&(dev->logFileIMUParsed), logBuf, logBufLen);

	return packetEnd + 3;

} // VN200IMUParse(VN200_DEV *, IMU_DATA *)

