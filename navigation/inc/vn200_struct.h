/***************************************************************************\
 *
 * File:
 * 	vn200_struct.h
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

#include "utils.h"
#include "buffer.h"
#include "logger.h"


typedef struct {
	double time;      // 0: Time of the week in seconds
	uint16_t week;    // 1: GPS week
	uint8_t GpsFix;   // 2: GPS fix type. 03 means locked.
	uint8_t NumSats;  // 3: Number of GPS satellites used in solution.
	double PosX;      // 4: ECEF X position in meters.
	double PosY;      // 5: ECEF Y position in meters.
	double PosZ;      // 6: ECEF Z position in meters.
	float VelX;       // 7: Velocity measurement in ECEF X direction.
	float VelY;       // 8: Velocity measurement in ECEF Y direction.
	float VelZ;       // 9: Velocity measurement in ECEF Z direction.
	float PosAccX;    // 10: ECEF X position accuracy estimate.
	float PosAccY;    // 11: ECEF Y position accuracy estimate.
	float PosAccZ;    // 12: ECEF Z position accuracy estimate.
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

typedef struct {

	int fd; // UART file descriptor

	BYTE_BUFFER inbuf;  // Input data buffer
	BYTE_BUFFER outbuf; // Output data buffer

	LOG_FILE logFile;           // Raw data log file
	LOG_FILE logFileGPSParsed;  // Parsed GPS data log file
	LOG_FILE logFileIMUParsed;  // Parsed IMU data log file

	int baud; // Baud rate
	int fs; // Sampling Frequency

} VN200_DEV;

#endif

