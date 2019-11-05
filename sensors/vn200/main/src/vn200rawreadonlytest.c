/***************************************************************************\
 *
 * File:
 * 	vn200rawreadonlytest.c
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
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <time.h>

#include "control.h"
#include "debuglog.h"
#include "logger.h"
#include "uart.h"
#include "VN200.h"


int main(void) {

	int numRead, numParsed, numConsumed, i;
	double time;

	LOG_FILE log;

	// Initialize log
	LogInit(&log, "log/SampleData/raw", "VN200", LOG_FILEEXT_LOG);

	const int buflen = 16384;
	char buf[buflen];

	int fd = UARTInitReadOnly(VN200_DEVNAME, 115200);
	if(fd < 0) {
		printf("Failed to open UART device\n");
		return 1;
	}

	int timeout = 0;

	// while(1) {
	int cumulative = 0;
	while(cumulative < 100000 && timeout < 1000) {

		getTimestamp(NULL, &time);
		logDebug("\nCurrent Time: %lf\n", time);

		numRead = UARTRead(fd, buf, buflen);
		cumulative += numRead;
		logDebug("Read %d bytes from UART\n", numRead);

		if(numRead > 0) {

			logDebug("\tRaw Data:\n");
			for(i = 0; i < numRead; i++) {
				logDebug("%c", buf[i]);
			}

			LogUpdate(&log, buf, numRead);

			timeout = 0;

		} else {
			timeout++;
		}

		usleep(10);

	}

	UARTClose(fd);

	return 0;

}

