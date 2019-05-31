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

#include "VN200.h"
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
#include <time.h>
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


/**** Function VN200GPSParse ****
 *
 * Parses data from VN200 input buffer, assuming device is configured as GPS
 *
 * Example packet:
 * 	$VNGPS,342123.000168,1890,3,05,+34.61463270,-112.45087270,+01559.954,+000.450,+000.770,-001.290,+002.940,+005.374,+007.410,+001.672,2.10E-08*23
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
int VN200GPSParse(char *buf, int len, GPS_DATA *data) {

	// Make extra sure there is enough room in the buffer
#define PACKET_BUF_SIZE 1024
	char currentPacket[PACKET_BUF_SIZE], logBuf[512];

	unsigned char chkOld, chkNew;
	int packetStart, packetEnd, logBufLen, i, rc;
	struct timespec timestamp_ts;

	// Exit on error if invalid pointer
	if(buf == NULL || data == NULL) {
		return -1;
	}

	packetStart = 0;
	// while(!valid) {

	/* Assume start of packet is 0
	// Find start of a packet ($)
	for( ; packetStart < dev->inbuf.length && 
		strncmp(&(dev->inbuf.buffer[packetStart]), "$VNGPS", 6); 
		packetStart++) {

		// printf("start: %d, strncmp is %d\n", packetStart, strncmp(&(dev->inbuf.buffer[packetStart]), "$VNGPS", 6));
	}
	*/

	// Find end of packet (*)
	for(packetEnd = packetStart; packetEnd < dev->inbuf.length - 3 && 
		dev->inbuf.buffer[packetEnd] != '*'; packetEnd++) ;

	if(packetStart >= len - 3 || packetEnd >= len - 3) {
		return 0;
	}

	if(packetEnd - packetStart > PACKET_BUF_SIZE - 1) {
		return -3;
	}

	// printf("Packet (start, end): %d %d\n", packetStart, packetEnd);
	// printf("                     %c %c\n", dev->inbuf.buffer[packetStart], dev->inbuf.buffer[packetEnd]);

	// Verify checksum
	// printf("Reading checksum\n");
	sscanf(&(dev->inbuf.buffer[packetEnd + 1]), "%hhX", &chkOld);
	chkNew = calculateChecksum(&(dev->inbuf.buffer[packetStart + 1]), packetEnd - packetStart - 1);
	// printf("Checksum (read, computed): %02X, %02X\n", chkOld, chkNew);

	if(chkNew != chkOld) {
		// Checksum failed, don't parse (but allow to skip to next packet)
		return 1;
	}

	// Make timestamp
	rc = clock_gettime(CLOCK_REALTIME, &timestamp_ts);
	if(rc) {
		perror("VN200GPSParse: Couldn't get timestamp");

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


	// Copy string into local variable
	strncpy(currentPacket, &(dev->inbuf.buffer[packetStart]), 
			packetEnd - packetStart);

	// Scan for fields in packet string
	rc = sscanf(currentPacket, "$VNGPS,%lf,%hd,%hhd,%hhd,%lf,%lf,%lf,%f,%f,%f,%f,%f,%f,%f,%f",
		&(data->time), &(data->week), &(data->GpsFix), &(data->NumSats),
		&(data->Latitude), &(data->Longitude), &(data->Altitude),
		&(data->NedVelX), &(data->NedVelY), &(data->NedVelZ),
		&(data->NorthAcc), &(data->EastAcc), &(data->VertAcc), &(data->SpeedAcc), &(data->TimeAcc));
	if(rc < 15) {
		printf("VN200GPSParse: Didn't match entire formatted string: %d\n", rc);
	}

	return packetEnd + 3;

} // VN200GPSParse(VN200_DEV *, GPS_DATA *)

int VN200GPSLogParsed(VN200_DEV *dev, GPS_DATA *data) {

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
	LogUpdate(&(dev->logFileGPSParsed), logBuf, logBufLen);

	return 0;

} // VN200GPSLogParsed(VN200_DEV *, GPS_DATA *)

