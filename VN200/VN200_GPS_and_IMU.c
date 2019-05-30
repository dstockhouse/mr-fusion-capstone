/***************************************************************************\
 *
 * File:
 * 	VN200_GPS_and_IMU.c
 *
 * Description:
 * 	Interfaces with a VN200 device outputting both GPS and IMU data
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/30/2019
 *
\***************************************************************************/

#include "VN200.h"
#include "VN200_GPS.h"
#include "VN200_IMU.h"
#include "VN200_GPS_and_IMU.h"

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


/**** Function VN200Init ****
 *
 * Initializes a VN200 UART device for both GPS and IMU functionality
 *
 * Arguments: 
 * 	dev  - Pointer to VN200_DEV instance to initialize
 * 	baud - Baud rate to configure the UART
 * 	fs   - Sampling frequency to initialize the module to
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int VN200Init(VN200_DEV *dev, int baud, int fs, int mode) {

#define CMD_BUFFER_SIZE 64
	char commandBuf[CMD_BUFFER_SIZE], logBuf[256];
	int commandBufLen, logBufLen;

	char logFileDirName[512];
	int logFileDirNameLength;

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	// Ensure valid init mode
	if(!(mode == VN200_INIT_MODE_GPS ||
	     mode == VN200_INIT_MODE_IMU ||
	     mode == VN200_INIT_MODE_BOTH)) {

		printf("VN200Init: Mode %d not recognized.\n", mode);
		return -2;
	}

	// Ensure valid sample rate (later)



	// Initialize UART for all modes
	VN200BaseInit(dev, baud);

	// Initialize log file for raw and parsed data
	// Since multiple log files will be generated for the run, put them in
	// the same directory
	logFileDirNameLength = generateFilename(logFileDirName, 512,
			"../SampleData/VN200", "run", "d");
	LogInit(&(dev->logFile), logFileDirName, "VN200", LOG_FILEEXT_LOG);

	// If GPS enabled, init GPS log file
	if(mode & VN200_INIT_MODE_GPS) {

		// Init csv file
		LogInit(&(dev->logFileGPSParsed), logFileDirName, "VN200_GPS", LOG_FILEEXT_CSV);

		// Write header to CSV data
		logBufLen = snprintf(logBuf, 256, "gpstime,week,gpsfix,numsats,lat,lon,alt,velx,vely,velz,nacc,eacc,vacc,sacc,tacc,timestamp\n");
		LogUpdate(&(dev->logFileGPSParsed), logBuf, logBufLen);

	}
	
	// If IMU enabled, init IMU log file
	if(mode & VN200_INIT_MODE_IMU) {

		// Init csv file
		LogInit(&(dev->logFileIMUParsed), logFileDirName, "VN200_IMU", LOG_FILEEXT_CSV);

		// Write header to CSV data
		logBufLen = snprintf(logBuf, 256, "compx,compy,compz,accelx,accely,accelz,gyrox,gyroy,gyroz,temp,baro,timestamp\n");
		LogUpdate(&(dev->logFileIMUParsed), logBuf, logBufLen);

	}



	/**** Initialize VN200 using commands ****/

	// Request IMU serial number
	commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNRRG,03");
	VN200Command(dev, commandBuf, commandBufLen, 1);
	usleep(100000);

	// Disable asynchronous output
	commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,06,0");
	VN200Command(dev, commandBuf, commandBufLen, 1);
	usleep(100000);

	// Set sampling frequency
	dev->fs = fs;
	commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s%d", "VNWRG,07,", dev->fs);
	VN200Command(dev, commandBuf, commandBufLen, 1);
	usleep(100000);


	if(mode == VN200_INIT_MODE_GPS) {

		// Enable asynchronous GPS data output
		commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,06,20");

	} else if(mode == VN200_INIT_MODE_IMU) {

		// Enable async IMU Measurements on VN200
		commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,06,19");

	} else {

		// Enable both GPS and IMU output
		commandBufLen = snprintf(commandBuf, CMD_BUFFER_SIZE, "%s", "VNWRG,06,248");

	}


	// Send mode command to UART
	VN200Command(dev, commandBuf, commandBufLen, 1);
	usleep(100000);

	// Clear input buffer (temporary)
	VN200FlushInput(dev);

	return 0;

} // VN200Init(VN200_DEV *, int)


/**** Function VN200Parse ****
 *
 * Parses data from VN200 input buffer
 *
 * Arguments: 
 * 	dev  - Pointer to VN200_DEV instance to parse from
 * 	data - Pointer to GPS_DATA instance to store parsed data
 *
 * Return value:
 *	On success, returns number of bytes parsed from buffer
 *	On failure, returns a negative number
 */
int VN200Parse(VN200_DEV *dev, GPS_DATA *data) {

	// Make extra sure there is enough room in the buffer
#define PACKET_BUF_SIZE 1024
	char currentPacket[PACKET_BUF_SIZE], logBuf[512];

#define NUM_GPS_FIELDS 15
	char *tokenList[NUM_GPS_FIELDS];

	unsigned char chkOld, chkNew;
	int packetStart, packetEnd, logBufLen, i, rc;
	struct timespec timestamp_ts;

	// Exit on error if invalid pointer
	if(dev == NULL || data == NULL) {
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
		strncmp(&(dev->inbuf.buffer[packetStart]), "$VNGPS", 6); 
		packetStart++) {

		// printf("start: %d, strncmp is %d\n", packetStart, strncmp(&(dev->inbuf.buffer[packetStart]), "$VNGPS", 6));
	}

	// Find end of packet (*)
	for(packetEnd = packetStart; packetEnd < dev->inbuf.length - 3 && 
		dev->inbuf.buffer[packetEnd] != '*'; packetEnd++) ;

	if(packetStart >= dev->inbuf.length - 3 || packetEnd >= dev->inbuf.length - 3) {
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


	// Copy string to be modified by strtok
	strncpy(currentPacket, &(dev->inbuf.buffer[packetStart+7]), 
			packetEnd - packetStart - 7);

	// Parse between commas (not exactly safe)
	tokenList[0] = strtok(currentPacket, ",");
	for(i = 1; i < NUM_GPS_FIELDS && tokenList[i-1] != NULL; i++) {
		tokenList[i] = strtok(NULL, ",");
	}
	/*
	printf("Read %d GPS comma delimited fields from %d characters:\n", i-1, packetEnd);
	for(i = 0; i < NUM_GPS_FIELDS && tokenList[i] != NULL; i++) {
		printf(tokenList[i]);
		printf("\n");
	}
	*/

	// Get time
	sscanf(tokenList[0], "%lf", &(data->time));

	// Get week
	sscanf(tokenList[1], "%hd", &(data->week));

	// Get GPS fix
	sscanf(tokenList[2], "%hhd", &(data->GpsFix));

	// Get number of GPS satellites
	sscanf(tokenList[3], "%hhd", &(data->NumSats));

	// Get Latitude
	sscanf(tokenList[4], "%lf", &(data->Latitude));

	// Get Longitude
	sscanf(tokenList[5], "%lf", &(data->Longitude));

	// Get Altitude
	sscanf(tokenList[6], "%lf", &(data->Altitude));

	// Get NedVelX
	sscanf(tokenList[7], "%f", &(data->NedVelX));

	// Get NedVelY
	sscanf(tokenList[8], "%f", &(data->NedVelY));

	// Get NedVelZ
	sscanf(tokenList[9], "%f", &(data->NedVelZ));

	// Get North Accuracy
	sscanf(tokenList[10], "%f", &(data->NorthAcc));

	// Get East Accuracy
	sscanf(tokenList[11], "%f", &(data->EastAcc));

	// Get Vert Accuracy
	sscanf(tokenList[12], "%f", &(data->VertAcc));

	// Get Speed Accuracy
	sscanf(tokenList[13], "%f", &(data->SpeedAcc));

	// Get Time Accuracy
	sscanf(tokenList[14], "%f", &(data->TimeAcc));

	// Log parsed data to file in CSV format
	logBufLen = snprintf(logBuf, 512, "%.6lf,%hd,%hhd,%hhd,%.8lf,%.8lf,%.3lf,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.11f,%.9lf\n",
			data->time, data->week, data->GpsFix, data->NumSats,
			data->Latitude, data->Longitude, data->Altitude,
			data->NedVelX, data->NedVelY, data->NedVelZ,
			data->NorthAcc, data->EastAcc, data->VertAcc,
			data->SpeedAcc, data->TimeAcc, data->timestamp);

	LogUpdate(&(dev->logFileParsed), logBuf, logBufLen);

	return packetEnd + 3;

} // VN200Parse(VN200_DEV *, GPS_DATA *)

