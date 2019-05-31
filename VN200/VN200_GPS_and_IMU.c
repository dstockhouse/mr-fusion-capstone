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
 * Parses data from VN200 input buffer and determine packet type
 *
 * Arguments: 
 * 	dev  - Pointer to VN200_DEV instance to parse from
 * 	data - Pointer to GPS_DATA instance to store parsed data
 *
 * Return value:
 *	On success, returns number of bytes parsed from buffer
 *	On failure, returns a negative number
 */
int VN200Parse(VN200_DEV *dev) {

	// Make extra sure there is enough room in the buffer
#define PACKET_BUF_SIZE 1024
	char currentPacket[PACKET_BUF_SIZE], logBuf[512], packetID[16];

	unsigned char chkOld, chkNew;
	int packetIDLength, packetIndex, logBufLen, numParsed = 0, i, rc;
	struct timespec timestamp_ts;

	VN200_PACKET *packet; // Pointer to the current packet being parsed
	VN200_PACKET_RING_BUFFER *ringbuf;


	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	// Make local ringbuf pointer
	ringbuf = &(dev->ringbuf);

	// Loop through all packets in ring buffer
	for(packetIndex = ringbuf->start; packetIndex != ringbuf->end;
			packetIndex = (packetIndex + 1) % VN200_PACKET_RING_BUFFER_SIZE) {

		// Set up pointer to current packet
		packet = &(ringbuf->packets[packetIndex]);

		// If packet is incomplete or already parsed, do nothing and return
		if(VN200PacketIncomplete(packet)) {
			return numParsed;
		}

		// Only parse if hasn't already been parsed
		if(!(packet->isParsed)) {

			// Search for end of packet ID (ex. VNGPS)
			for(i = packet->startIndex; i < packet->endIndex && ringbuf->buf->buffer[i] != ','; i++) {
				// Loop until comma is reached
			}

			// Length of packet ID string is the number travelled
			packetIDLength = i - packet->startIndex;


			// Determine type of packet and parse accordingly

			// Packet is a GPS packet
			if(packetIDLength == 6 && 
					strncmp(ringbuf->buf->buffer[packet->startIndex], "$VNGPS", packetIDLength)) {

				// Parse as GPS packet
				rc = VN200GPSParse(&(ringbuf->buf->buffer[packet->startIndex]), &(packet->GPSData));

				// Set packet stats
				packet->contentsType = VN200_PACKET_CONTENTS_TYPE_GPS;
				packet->isParsed = 1;


			// Packet is an IMU packet
			} else if(packetIDLength == 6 && 
					strncmp(ringbuf->buf->buffer[packet->startIndex], "$VNIMU", packetIDLength)) {

				// Parse as IMU packet
				rc = VN200IMUParse(&(ringbuf->buf->buffer[packet->startIndex]), &(packet->IMUData));

				// Set packet stats
				packet->contentsType = VN200_PACKET_CONTENTS_TYPE_IMU;
				packet->isParsed = 1;


			// Packet is unknown type or improperly formatted
			} else {

				char tempbuf[512];

				// Printout confusion message
				printf("Unknown or improper message: ");

				snprintf(tempbuf, "%s", &(ringbuf->buf->buffer[packet->startIndex]), MIN(512, packet->endIndex - packet->startIndex));
				printf(tempbuf);
				printf("\n");

				// Set packet stats
				packet->contentsType = VN200_PACKET_CONTENTS_TYPE_OTHER;
				packet->isParsed = 1;

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
		// Checksum failed, don't parse but skip to next packet
		return 1;
	}

	// Log parsed data to file in CSV format
	logBufLen = snprintf(logBuf, 512, "%.6lf,%hd,%hhd,%hhd,%.8lf,%.8lf,%.3lf,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.11f,%.9lf\n",
			data->time, data->week, data->GpsFix, data->NumSats,
			data->Latitude, data->Longitude, data->Altitude,
			data->NedVelX, data->NedVelY, data->NedVelZ,
			data->NorthAcc, data->EastAcc, data->VertAcc,
			data->SpeedAcc, data->TimeAcc, data->timestamp);

	LogUpdate(&(dev->logFileParsed), logBuf, logBufLen);

		} // if packet not already parsed

	} // for packets in ring buffer

	return packetEnd + 3;

} // VN200Parse(VN200_DEV *, GPS_DATA *)

int VN200PacketParse(VN200_PACKET_RING_BUFFER *ringbuf, int packetOffset) {

	int unrolledEnd;

	if(ringbuf == NULL) {
		return -1;
	}

	// Compute unrolled end index (without modding)
	unrolledEnd = ringbuf->end;
	if(ringbuf->end < ringbuf->start) {
		unrolledEnd += VN200_PACKET_RING_BUFFER_SIZE;
	}

	// If start+offset past the end, invalid offset so stop
	if(ringbuf->start + packetOffset >= unrolledEnd) {
		return -2;
	}

}

int VN200PacketRingBufferEmpty(VN200_PACKET_RING_BUFFER *ringbuf) {

	ringbuf->start = 0;
	ringbuf->end = 0;

	return 0;

} // VN200PacketRingBufferEmpty(VN200_PACKET_RING_BUFFER *)

int VN200PacketRingBufferIsEmpty(VN200_PACKET_RING_BUFFER *ringbuf) {

	if(ringbuf == NULL) {
		return -1;
	}

	return 0;

} // VN200PacketRingBufferIsEmpty(VN200_PACKET_RING_BUFFER *)

int VN200PacketRingBufferIsFull(VN200_PACKET_RING_BUFFER *ringbuf) {

	if(ringbuf == NULL) {
		return -1;
	}

	// Full if end is one behind start
	return ringbuf->end == (ringbuf->end - 1) % 1;

} // VN200PacketRingBufferIsEmpty(VN200_PACKET_RING_BUFFER *)

int VN200PacketRingBufferAddPacket(VN200_PACKET_RING_BUFFER *ringbuf, int startIndex, int endIndex) {

	int packetIndex;

	if(ringbuf == NULL) {
		return -1;
	}

	if(VN200PacketRingBufferIsFull(ringbuf)) {
		return 0;
	}

	packetIndex = ringbuf->end;

	// Move end by one
	ringbuf->end = (ringbuf->end + 1) % VN200_RING_BUFFER_SIZE;

	// Get timestamp
	getTimestamp(&(ringbuf->packets[packetIndex].timestamp_ts), &(ringbuf->packets[packetIndex].timestamp));

	// Set contents type to neither GPS nor IMU
	ringbuf->packets[packetIndex].contentsType = VN200_PACKET_CONTENTS_TYPE_OTHER;

	// Set not parsed
	ringbuf->packets[packetIndex].isParsed = 0;

	// Set packet start and end point
	ringbuf->packets[packetIndex].startIndex = startIndex;
	ringbuf->packets[packetIndex].endIndex = endIndex;

	return 1;

} // VN200PacketRingBufferAddPacket(VN200_PACKET_RING_BUFFER *, int, int)

int VN200PacketRingBufferUpdateEndpoint(VN200_PACKET_RING_BUFFER *ringbuf) {

	int lastPacketIndex, i;

	if(ringbuf == NULL) {
		return -1;
	}

	if(VN200PacketRingBufferIsEmpty(ringbuf)) {
		return 0;
	}

	// Find last packet in ring buffer
	lastPacketIndex = (ringbuf->end - 1) % VN200_PACKET_RING_BUFFER_SIZE;

	// Loop from current ring-buffer-known end to true end of buffer
	for(i = ringbuf->packets[lastPacketIndex].endIndex; i < ringbuf->buf->length; i++) {

		// Determine if start of new packet
		if(ringbuf->buf->buffer[i] == '$') {

			// End previous packet
			ringbuf->packets[lastPacketIndex].endIndex = i;

			// Create new packet
			VN200PacketRingBufferAddPacket(ringbuf, i, i);

			// Update last packet index
			lastPacketIndex = (ringbuf->end - 1) % VN200_PACKET_RING_BUFFER_SIZE;

		}

	}

	return 0;

} // VN200PacketRingBufferUpdateEndpoint(VN200_PACKET_RING_BUFFER *) {

int VN200PacketIncomplete(VN200_PACKET *packet) {

	if(ringbuf == NULL) {
		return -1;
	}

	// Packet is incomplete if the 
	return packet->startIndex == packet->endIndex;

} // VN200PacketIncomplete(VN200_PACKET *)

