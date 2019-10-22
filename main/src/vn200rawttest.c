/***************************************************************************\
 *
 * File:
 * 	vn200rawtest.c
 *
 * Description:
 *	Tests the VN200 combined functionality, logging data without parsing
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 9/19/2019
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "buffer.h"
#include "logger.h"
#include "uart.h"
#include "VN200.h"


int main(void) {

	int numRead, numParsed, numConsumed, i;

	// Instances of structure variables
	VN200_DEV dev;

	logDebug("Initializing...\n");
	VN200Init(&dev, 50, 115200, VN200_INIT_MODE_BOTH);

	while(1) {

		numRead = VN200Poll(&dev);
#ifdef STANDARD_DEBUG
		logDebug("Read %d bytes from UART\n", numRead);
#endif

		do {

			numParsed = VN200Parse(&dev, &data);
#ifdef STANDARD_DEBUG
			logDebug("Parsed %d bytes from buffer\n", numParsed);
#endif

#ifdef VERBOSE_DEBUG
			if(numParsed > 0) {

				logDebug("\tRaw Data:\n");
				logDebug("\t\t");
				for(i = 0; i < numParsed; i++) {
					logDebug("%c", dev.inbuf.buffer[i]);
				}
			}
#endif

			numConsumed = VN200Consume(&dev, numParsed);
#ifdef STANDARD_DEBUG
			logDebug("Consumed %d bytes from buffer\n", numConsumed);
#endif


		} while(numConsumed > 0);

		usleep(10);

	}

	VN200Destroy(&dev);

	return 0;

}

