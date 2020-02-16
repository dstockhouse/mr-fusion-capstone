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

	return VN200Init(dev, fs, VN200_BAUD, VN200_INIT_MODE_IMU);

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
int VN200IMUPacketParse(char *buf, int len, IMU_DATA *data) {

	// 1K Should be enough for a single packet
	const int PACKET_BUF_SIZE = 1024;
	char currentPacket[PACKET_BUF_SIZE];
	int i, rc;

	// Exit on error if invalid pointer
	if(buf == NULL || data == NULL) {
		return -1;
	}

	// Exit if invalid length
	if(len < 0) {
		return -2;
	}

	logDebug(L_VDEBUG, "\n\n%s - Data in buffer:\n", __func__);
	for(i = 0; i < len; i++) {
		logDebug(L_VDEBUG, "%c", buf[i]);
	}
	logDebug(L_VDEBUG, "\n\n");

	// Copy string into local variable
	strncpy(currentPacket, &(buf[0]), len);

	// Parse out values (all doubles)
	rc = sscanf(currentPacket, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
			&(data->compass[0]), &(data->compass[1]), &(data->compass[2]),
			&(data->accel[0]), &(data->accel[1]), &(data->accel[2]),
			&(data->gyro[0]), &(data->gyro[1]), &(data->gyro[2]),
			&(data->temp), &(data->baro));
	if(rc < 11) {
		logDebug("%s: Didn't match entire formatted string: %d\n", __func__, rc);

		// Malformed packet, return error to indicate not fully parsed
		return -3;
	}

	return len;

} // VN200IMUPacketParse(char *, int, IMU_DATA *) {


int VN200IMULogParsed(LOG_FILE *log, IMU_DATA *data) {

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

