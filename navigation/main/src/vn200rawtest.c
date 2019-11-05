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

	struct timespec delaytime;
	int starttime;

	logDebug("Initializing...\n");

	VN200Init(&dev, 50, VN200_BAUD, VN200_INIT_MODE_BOTH);
	sleep(1);

	char *command = "VNWRG,06,248";
	VN200Command(&dev, command, strlen(command), 0);
	VN200FlushOutput(&dev);

	// while(1) {
	int cumulative = 0;
	while(cumulative < 100000) {

		getTimestamp(NULL, &time);
#ifdef VERBOSE_DEBUG
		logDebug("Current Time: %lf\n", time);
#endif

		numRead = VN200Poll(&dev);
		cumulative += numRead;
#ifdef STANDARD_DEBUG
		logDebug("Read %d bytes from UART\n", numRead);
#endif

#ifdef VERBOSE_DEBUG
		if(numRead > 0) {

			logDebug("\tRaw Data:\n");
			logDebug("\t\t");
			for(i = 0; i < numRead; i++) {
				logDebug("%c", dev.inbuf.buffer[i]);
			}
		}
#endif

#if 0
		do {

			numParsed = VN200Parse(&dev, &data);
#ifdef STANDARD_DEBUG
			logDebug("Parsed %d bytes from buffer\n", numParsed);
#endif

			numConsumed = VN200Consume(&dev, numParsed);


		} while(numConsumed > 0);
#endif

		numConsumed = VN200Consume(&dev, numRead);
#ifdef STANDARD_DEBUG
		logDebug("Consumed %d bytes from buffer\n", numConsumed);
#endif

		usleep(10);

	}

	VN200Destroy(&dev);

	return 0;

}

