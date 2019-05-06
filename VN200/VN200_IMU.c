/****************************************************************************\
 *
 * File: 
 * 	VN200_IMU.c
 *
 * Description: 
 * 	Initialize the VN200 to gather and send IMU data
 *
 * Author:
 * 	Joseph Kroeker
 *
 * Revision 0.1
 * 	Last edited 4/1/2019
 *
\****************************************************************************/

#include "uart.h"
#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>

/**** Function VN200IMUInit ****
 *
 * Initialize the VN200 for IMU data
 *
 * Arguments: None 
 *
 * Return value:
 * 	Return 0 on success
 * 	Return a negative number on failure
 */
int VN200IMUInit(VN200_IMU *dev);

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	dev->fd = UARTInit(VN200_IMU_DEV, VN200_IMU_BAUD);
	if(dev->fd < 0) {
		printf("Couldn't initialize VN200 IMU UART device (%s)\n", VN200_IMU_DEV);
		return -2;
	}

	// Initialize the input buffer
	BufferEmpty(&(dev->inbuf));

	// Initialize log file
	LogInit(&(dev->logFile), "SampleData/VN200_IMU", "VN200_IMU", 1);
	
	// Request IMU serial number
	UARTWrite(dev->outbuf, "VNRRG,03")

	return 0;

} // VN200IMUInit(VN200_IMU *)

