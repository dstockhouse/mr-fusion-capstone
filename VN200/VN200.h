/***************************************************************************\
 *
 * File:
 * 	VN200.h
 *
 * Description:
 *	Function and type declarations and constants for VN200.c
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/06/2019
 *
 * Revision 0.2
 * 	Last edited 5/30/2019
 * 	Major overhaul unifying GPS and IMU functionality
 *
 ***************************************************************************/

#ifndef __VN200_H
#define __VN200_H

#include "../uart/uart.h"
#include "../buffer/buffer.h"
#include "../logger/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#define VN200_DEVNAME "/dev/ttyUSB0"
#define VN200_BAUD 57600
//#define VN200_BAUD 115200


/* Old method 
typedef struct {
	int fd;
	BYTE_BUFFER inbuf;
	BYTE_BUFFER outbuf;
	LOG_FILE logFile;
	LOG_FILE logFileParsed;
	int baud; // Baud rate 115200, 128000, 230400, 460800, 921600
	int fs; // Sampling Frequency => 100
} VN200_DEV;
*/

typedef struct {

	int fd; // UART file descriptor

	BYTE_BUFFER packetbuf;  // Input data buffer
	BYTE_BUFFER outbuf;     // Output data buffer
	VN200_PACKET_RING_BUFFER ringbuf; // Ring buffer for input packet data

	LOG_FILE logFile; // Raw data log file
	LOG_FILE logFileGPSParsed; // Parsed GPS data log file
	LOG_FILE logFileIMUParsed; // Parsed IMU data log file

	int baud; // Baud rate
	int fs; // Sampling Frequency

} VN200_DEV;

int getTimestamp(struct timespec *ts, double *td);

int VN200BaseInit(VN200_DEV *dev);

int VN200Poll(VN200_DEV *dev);

int VN200Consume(VN200_DEV *dev, int num);

int VN200FlushInput(VN200_DEV *dev);

int VN200Command(VN200_DEV *dev, char *buf, int num, int sendChk);

int VN200FlushOutput(VN200_DEV *dev);

int VN200Destroy(VN200_DEV *dev);

#endif

