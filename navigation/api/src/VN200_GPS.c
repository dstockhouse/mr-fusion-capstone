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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
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
#include "VN200_GPS.h"
#include "VN200.h"


/**** Function VN200GPSInit ****
 *
 * Initializes a VN200 UART device for GPS functionality
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

	return VN200Init(dev, fs, VN200_BAUD, VN200_INIT_MODE_GPS);

} // VN200GPSInit(VN200_DEV *, int)


/**** Function VN200GPSPacketParse ****
 *
 * Parses data from input buffer, assuming GPS
 *
 * Example packet:
 * 	$VNGPS,342123.000168,1890,3,05,+34.61463270,-112.45087270,+01559.954,+000.450,+000.770,-001.290,+002.940,+005.374,+007.410,+001.672,2.10E-08*23
 *
 * 	The input buffer must start at first character after "$VNGPS," and the
 * 	length is the number of characters until *XX
 *
 * Arguments: 
 * 	buf  - Pointer to character buffer to parse from
 * 	len  - Length of character buffer
 * 	data - Pointer to GPS_DATA instance to store parsed data
 *
 * Return value:
 *	On success, returns number of bytes parsed from buffer
 *	On failure, returns a negative number
 */
int VN200GPSPacketParse(char *buf, int len, GPS_DATA *data) {

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

#ifdef VERBOSE_DEBUG
	logDebug("\n\n%s - Data in buffer:\n", __func__);
	for(i = 0; i < len; i++) {
		logDebug("%c", buf[i]);
	}
	logDebug("\n\n");
#endif

	// Copy string into local variable
	strncpy(currentPacket, &(buf[0]), len);

	// Scan for fields in packet string
	rc = sscanf(currentPacket, "%lf,%hd,%hhd,%hhd,%lf,%lf,%lf,%f,%f,%f,%f,%f,%f,%f,%f",
		&(data->time), &(data->week), &(data->GpsFix), &(data->NumSats),
		&(data->Latitude), &(data->Longitude), &(data->Altitude),
		&(data->NedVelX), &(data->NedVelY), &(data->NedVelZ),
		&(data->NorthAcc), &(data->EastAcc), &(data->VertAcc), &(data->SpeedAcc), &(data->TimeAcc));
	if(rc < 15) {
		logDebug("%s: Didn't match entire formatted string: %d\n", __func__, rc);

		// Malformed packet, return 1 to indicate not fully parsed
		return 1;
	}

	return len;

} // VN200GPSPacketParse(char *, int, GPS_DATA *)


int VN200GPSLogParsed(LOG_FILE *log, GPS_DATA *data) {

	char logBuf[512];
	int logBufLen;

	// Log parsed data to file in CSV format
	logBufLen = snprintf(logBuf, 512, "%.6lf,%hd,%hhd,%hhd,%.8lf,%.8lf,%.3lf,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.11f,%.9lf\n",
			data->time, data->week, data->GpsFix, data->NumSats,
			data->Latitude, data->Longitude, data->Altitude,
			data->NedVelX, data->NedVelY, data->NedVelZ,
			data->NorthAcc, data->EastAcc, data->VertAcc,
			data->SpeedAcc, data->TimeAcc, data->timestamp);

	// Update log file
	LogUpdate(log, logBuf, logBufLen);

	return 0;

} // VN200GPSLogParsed(VN200_DEV *, GPS_DATA *)

