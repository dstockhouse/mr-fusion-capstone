/***************************************************************************\
 *
 * File:
 * 	VN200_IMU.h
 *
 * Description:
 *	Function and type declarations and constants for VN200_IMU.c
 * 
 * Author:
 * 	Joseph Kroeker
 *
 * Revision 0.1
 * 	Last edited 4/1/2019
 *
 ***************************************************************************/


#define VN200_IMU_DEV "/dev/ttyUSB0"
#define VN200_IMU_BAUD 57600

typedef struct {
	int fd;
	BYTE_BUFFER inbuf;
	BYTE_BUFFER outbuf;
	LOG_FILE logFile;
} VN200_IMU;

int VN200IMUInit(VN200_IMU *dev);

