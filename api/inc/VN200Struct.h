/***************************************************************************\
 *
 * File:
 * 	VN200Struct.h
 *
 * Description:
 *	Structures containing device and data for VN200. Must be kept
 *	separate so that the order of includes doesn't get too complicated.
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 9/19/2019
 *
 ***************************************************************************/

#ifndef __VN200_STRUCT_H
#define __VN200_STRUCT_H

#include <inttypes.h>
#include <time.h>

#include "buffer.h"
#include "logger.h"


typedef struct {
	double time;      // 0: Time of the week in seconds
	uint16_t week;    // 1: GPS week
	uint8_t GpsFix;   // 2: GPS fix type. See table below.
	uint8_t NumSats;  // 3: Number of GPS satellites used in solution.
	double Latitude;  // 4: Latitude in degrees.
	double Longitude; // 5: Longitude in degrees.
	double Altitude;  // 6: Altitude above ellipsoid. (WGS84)
	float NedVelX;    // 7: Velocity measurement in north direction.
	float NedVelY;    // 8: Velocity measurement in east direction.
	float NedVelZ;    // 9: Velocity measurement in down direction.
	float NorthAcc;   // 10: North position accuracy estimate. (North)
	float EastAcc;    // 11: East position accuracy estimate. (East)
	float VertAcc;    // 12: Vertical position accuracy estimate. (Down)
	float SpeedAcc;   // 13: Speed accuracy estimate.
	float TimeAcc;    // 14: Time accuracy estimate.  

	double timestamp; // System time data was collected

} GPS_DATA;

typedef struct {
	double compass[3]; // compass (x,y,z) Gauss
	double accel[3]; // accel (x,y,z) m/s^2
	double gyro[3]; // gyro (x, y, z) rad/s
	double temp; // temp C
	double baro; // pressure kPa

	double timestamp; // System time data was collected

} IMU_DATA;


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


// Large enough to not worry about overrunning
#define VN200_PACKET_RING_BUFFER_SIZE 256

// Input buffer for data of multiple packets
typedef struct {

	BYTE_BUFFER *buf; // Pointer to buffer for input packet data

	// Buffer of packets
	VN200_PACKET packets[VN200_PACKET_RING_BUFFER_SIZE];

	int start; // Index of first valid packet element
	int end;   // Index after last valid element (to add next)
	           // Buffer is full if start == end - 1

} VN200_PACKET_RING_BUFFER;


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

	BYTE_BUFFER inbuf;  // Input data buffer
	BYTE_BUFFER outbuf;     // Output data buffer
	VN200_PACKET_RING_BUFFER ringbuf; // Ring buffer for input packet data

	LOG_FILE logFile; // Raw data log file
	LOG_FILE logFileGPSParsed; // Parsed GPS data log file
	LOG_FILE logFileIMUParsed; // Parsed IMU data log file

	int baud; // Baud rate
	int fs; // Sampling Frequency

} VN200_DEV;

#endif

