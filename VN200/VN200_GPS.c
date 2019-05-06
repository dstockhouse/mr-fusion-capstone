/****************************************************************************\
 *
 * File:
 * 	VN200_GPS.c
 *
 * Description:
 * 	Interfaces with an GPS reciever connected to serial port through USB
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 4/16/2019
 *
 * Revision 0.2
 * 	Last edited 5/06/2019
 *
\***************************************************************************/

#include "VN200_GPS.h"

#include "../uart/uart.h"
#include "../buffer/buffer.h"
#include "../logger/logger.h"
#include "crc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>


/**** Function VN200GPSInit ****
 *
 * Initializes a pingUSB UART receiver instance
 *
 * Arguments: 
 * 	dev - Pointer to VN200_DEV instance to initialize
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int VN200GPSInit(VN200_DEV *dev) {

#define CMD_BUFFER_SIZE 64
	char commandBuf[CMD_BUFFER_SIZE];
	int commandBufLen;

	// Exit on error if invalid pointer
	if(dev == NULL) {
		return -1;
	}

	VN200BaseInit(dev);

	commandBufLen = snprintf(commandBuf, "%s", "VNWRG,6,0", CMD_BUFFER_SIZE);
	VN200Command(dev, commandBuf, commandBufLen);

	snprintf(commandBuf, "%s%d", "VNWRG,7,", dev->fs, CMD_BUFFER_SIZE);
	VN200Command(dev, commandBuf, commandBufLen);

	snprintf(commandBuf, "%s", "VNWRG,6,20", CMD_BUFFER_SIZE);
	VN200Command(dev, commandBuf, commandBufLen);

	// Clear input buffer (temporary)
	VN200FlushInput(dev);

	return 0;

} // VN200GPSInit(VN200_DEV *)

