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
#include "VN200.h"

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

	// Initialize the input buffer
	BufferEmpty(&(dev->inbuf));

	// Initialize log file
	LogInit(&(dev->logFile), "SampleData/VN200_IMU", "VN200_IMU", 1);
	
	// Request IMU serial number
	commandBufLen = snprintf(commandBuf, "%s", "VNRRG,03", CMD_BUFFER_SIZE);
	VN200Command(dev, commandBuf, commandBufLen);
	delay(100); 
	VN200FlushInput(dev);

	// Disable asynchronous data output
	commandBufLen = snprintf(commandBuf, "%s", "VNWRG, 6,0", CMD_BUFFER_SIZE);
	VN200Command(dev, commandBuf, commandBufLen);
	
	// Set the asynchronous data output freq
	commandBufLen = snprintf(commandBuf, "VNWRG, 7, %d", dev->fs, CMD_BUFFER_SIZE);
	VN200Command(dev, commandBuf, commandBufLen);
	delay(100);
	VN200FlushInput(dev);

	// Enable async IMU Measurements on VN200
	commandBufLen = snprintf(commandBuf, "%s", "VNWRG, 6, 19", CMD_BUFFER_SIZE);
	VN200Command(dev, commandBuf, commandBufLen);
	delay(100);
	VN200FlushInput(dev);

	return 0;
} // VN200IMUInit(VN200_IMU *)

