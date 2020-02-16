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
#include <time.h>

#include "control.h"
#include "debuglog.h"
#include "buffer.h"
#include "logger.h"
#include "uart.h"
#include "VN200.h"


int main(void) {

	int numRead, numParsed, numConsumed, i;
	double time;

	// Instances of structure variables
	VN200_DEV dev;

	logDebug("Initializing...\n");

	struct timespec delaytime;
	int starttime;

	// Initialize as IMU at 50 Hz
	VN200Init(&dev, 50, VN200_BAUD, VN200_INIT_MODE_IMU);

	clock_gettime(CLOCK_REALTIME, &delaytime);
	starttime = delaytime.tv_sec;

	// while(1) {
	int cumulative = 0;
	while(cumulative < 100000 && delaytime.tv_sec < starttime + 6) {

		getTimestamp(NULL, &time);
		logDebug(L_VDEBUG, "Current Time: %lf\n", time);

		numRead = VN200Poll(&dev);
		cumulative += numRead;
		logDebug(L_DEBUG, "Read %d bytes from UART\n", numRead);

		if(numRead > 0) {

			logDebug(L_VDEBUG, "\tRaw Data:\n");
			logDebug(L_VDEBUG, "\t\t");
			for(i = 0; i < numRead; i++) {
				logDebug(L_VDEBUG, "%c", dev.inbuf.buffer[i]);
			}
		}

#if 0
		do {

			numParsed = VN200Parse(&dev, &data);
			logDebug(L_DEBUG, "Parsed %d bytes from buffer\n", numParsed);

			numConsumed = VN200Consume(&dev, numParsed);


		} while(numConsumed > 0);
#endif

		numConsumed = VN200Consume(&dev, numRead);
		logDebug(L_DEBUG, "Consumed %d bytes from buffer\n", numConsumed);

		usleep(10);

		clock_gettime(CLOCK_REALTIME, &delaytime);

	}

	VN200Destroy(&dev);

	return 0;

}

