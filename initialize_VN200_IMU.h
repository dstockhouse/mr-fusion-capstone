/***************************************************************************\
 *
 * File:
 * 	initialize_VN200_IMU.h
 *
 * Description:
 *	Function and type declarations and constants for initialize_VN200_IMU.c
 * 
 * Author:
 * 	Joseph Kroeker
 *
 * Revision 0.1
 * 	Last edited 4/1/2019
 *
 ***************************************************************************/


typedef struct {
	int fd;
	BYTE_BUFFER inbuf;
	BYTE_BUFFER outbuf;
	LOG_FILE logFile;
} USB_IMU;

int IMUInit(void);

int pingUSBInit(USB_IMU *dev);


