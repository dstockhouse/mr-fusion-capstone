/****************************************************************************\
 *
 * File:
 * 	VN200_GPS_and_IMU.h
 *
 * Description:
 *	Function and type declarations and constants for VN200_GPS_and_IMU.c
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/30/2019
 *
 ****************************************************************************/

#ifndef __VN200_GPS_H
#define __VN200_GPS_H

#include "VN200.h"

#include <inttypes.h>
#include <time.h>

// Large enough to not worry about overrunning
#define VN200_PACKET_RING_BUFFER_SIZE 64

#define VN200_INIT_MODE_GPS 1
#define VN200_INIT_MODE_IMU 2
#define VN200_INIT_MODE_BOTH (VN200_INIT_MODE_GPS|VN200_INIT_MODE_IMU)


// For indicating the type of data in a packet structure
typedef enum {
	VN200_PACKET_CONTENTS_TYPE_GPS,
	VN200_PACKET_CONTENTS_TYPE_IMU,
	VN200_PACKET_CONTENTS_TYPE_OTHER
} VN200_PACKET_CONTENTS_TYPE;


// A packet from the VN200 which may either be GPS, IMU, or other data.
// Includes the raw data in a buffer and after parsing
typedef struct {

	// Raw data bufer. Could be made smaller than BYTE_BUFFER in the future
	// for memory conservation
	BYTE_BUFFER buf;

	// Enum indicating type of data
	VN200_PACKET_CONTENTS_TYPE contentsType;

	GPS_DATA GPSData; // Parsed GPS data (if packet is GPS data)
	IMU_DATA IMUData; // Parsed IMU data (if packet is IMU data)

	int isParsed;     // Bool indicating that the raw data has been parsed
	double timestamp; // Timestamp when packet start was read from UART

	// Not yet implemented
	// semaphore_t *sem;  // Pointer to a semaphore (to use if multithreaded)

} VN200_PACKET;


// Ring buffer of multiple packets
typedef struct {

	int start; // Index of first valid element
	int end;   // Index after last valid element (to add next)
	           // Buffer is full if start == end - 1

	// Buffer of packets
	VN200_PACKET packets[VN200_PACKET_RING_BUFFER_SIZE];

} VN200_PACKET_RING_BUFFER;


int VN200Init(VN200_DEV *dev, int baud, int fs, int mode);

int VN200Parse(VN200_DEV *dev, GPS_DATA *parsedData);

#endif

