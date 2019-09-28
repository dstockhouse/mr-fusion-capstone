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

#include "control.h"
#include "debuglog.h"
#include "buffer.h"
#include "logger.h"
#include "uart.h"
#include "VN200_CRC.h"
#include "VN200_IMU.h"
#include "VN200.h"

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

	const int CMD_BUFFER_SIZE = 64;
	char commandBuf[CMD_BUFFER_SIZE], logBuf[256];
	int commandBufLen, logBufLen;

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	// Initialize UART for use
	VN200BaseInit(dev);

	// Initialize log file for raw and parsed data
	LogInit(&(dev->logFile), "../log/SampleData/VN200/IMU", "VN200", LOG_FILEEXT_LOG);
	LogInit(&(dev->logFileIMUParsed), "../log/SampleData/VN200/IMU", "VN200", LOG_FILEEXT_CSV);

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


****** Change to input single packet ptr, IMU_DATA ptr

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
int VN200IMUPacketParse(char *buf, int len, IMU_DATA *data) {

	// 1K Should be enough for a single packet
	const int PACKET_BUF_SIZE = 1024;
	char currentPacket[PACKET_BUF_SIZE];

	int i, rc;
	struct timespec timestamp_ts;

	// Exit on error if invalid pointer
	if(buf == NULL || data == NULL) {
		return -1;
	}

	// Exit if invalid length
	if(len < 0) {
		return -2;
	}

#ifdef VERBOSE_DEBUG
	logDebug("\n\n%s - Data in buffer:\n", __func__);
	for(i = 0; i < len; i++) {
		logDebug("%c", buf[i]);
	}
	logDebug("\n\n");
#endif

	// Copy string into local variable
	strncpy(currentPacket, &(buf[0]), len);

	// Parse out values (all doubles)
	sscanf(currentPacket, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
			&(data->compass[0]), &(data->compass[1]), &(data->compass[2]),
			&(data->accel[0]), &(data->accel[1]), &(data->accel[2]),
			&(data->gyro[0]), &(data->gyro[1]), &(data->gyro[2]),
			&(data->temp), &(data->baro));
	if(rc < 15) {
		logDebug("%s: Didn't match entire formatted string: %d\n", __func__, rc);

		// Malformed packet, return 1 to indicate not fully parsed
		return 1;
	}

	return len;

} // VN200IMUPacketParse(char *, int, IMU_DATA *) {


int VN200IMULogParsed(LOGFILE *log, IMU_DATA *data) {

	char logBuf[512];
	int logBufLen;

	// Log parsed data to file in CSV format
	logBufLen = snprintf(logBuf, 512, "%.4lf,%.4lf,%.4lf,%.3lf,%.3lf,%.3lf,%.6lf,%.6lf,%.6lf,%.1lf,%.3lf,%.9lf\n",
			data->compass[0], data->compass[1], data->compass[2],
			data->accel[0], data->accel[1], data->accel[2],
			data->gyro[0], data->gyro[1], data->gyro[2],
			data->temp, data->baro, data->timestamp);

	LogUpdate(log, logBuf, logBufLen);

	return 0;
}

