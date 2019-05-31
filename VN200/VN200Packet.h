/***************************************************************************\
 *
 * File:
 * 	VN200Packet.h
 *
 * Description:
 *	Function and type declarations and constants for VN200Packet.c
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/30/2019
 *
 ***************************************************************************/

#ifndef __VN200_PACKET_H
#define __VN200_PACKET_H

#include "../uart/uart.h"
#include "../buffer/buffer.h"
#include "../logger/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

// Large enough to not worry about overrunning
#define VN200_PACKET_RING_BUFFER_SIZE 256


// For indicating the type of data in a packet structure
typedef enum {
	VN200_PACKET_CONTENTS_TYPE_GPS,
	VN200_PACKET_CONTENTS_TYPE_IMU,
	VN200_PACKET_CONTENTS_TYPE_OTHER
} VN200_PACKET_CONTENTS_TYPE;

// A packet from the VN200 which may either be GPS, IMU, or other data.
// Includes the raw data in a buffer and after parsing
typedef struct {

	int startIndex; // Buffer offset for start of packet data
	int endIndex;   // Buffer offset for end of complete packet data
	// The end index is only set when entire packet data is available

	// Enum indicating type of data
	VN200_PACKET_CONTENTS_TYPE contentsType;

	GPS_DATA GPSData; // Parsed GPS data (if packet is GPS data)
	IMU_DATA IMUData; // Parsed IMU data (if packet is IMU data)

	int isParsed;     // Bool indicating that the raw data has been parsed
	double timestamp; // Timestamp when packet start was read from UART
	struct timespec timestamp_ts;

	// Not yet implemented
	// semaphore_t *sem;  // Pointer to a semaphore (to use if multithreaded)

} VN200_PACKET;

// Input buffer for data of multiple packets
typedef struct {

	BYTE_BUFFER *buf; // Pointer to buffer for input packet data

	// Buffer of packets
	VN200_PACKET packets[VN200_PACKET_RING_BUFFER_SIZE];

	int start; // Index of first valid packet element
	int end;   // Index after last valid element (to add next)
	           // Buffer is full if start == end - 1

} VN200_PACKET_RING_BUFFER;

int VN200PacketParse(VN200_PACKET_RING_BUFFER *ringbuf, int packetOffset);

int VN200PacketRingBufferEmpty(VN200_PACKET_RING_BUFFER *ringbuf);

int VN200PacketRingBufferIsEmpty(VN200_PACKET_RING_BUFFER *ringbuf);

int VN200PacketRingBufferAddPacket(VN200_PACKET_RING_BUFFER *ringbuf, int startIndex, int endIndex);

int VN200PacketRingBufferUpdateEndpoint(VN200_PACKET_RING_BUFFER *ringbuf);

int VN200PacketIncomplete(VN200_PACKET *packet);


#endif

