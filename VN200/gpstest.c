/***************************************************************************\
 *
 * File:
 * 	gpstest.c
 *
 * Description:
 *	Tests the VN200 GPS functionality
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/07/2019
 *
 ***************************************************************************/

#include "VN200.h"
#include "VN200_GPS.h"

#include "../uart/uart.h"
#include "../buffer/buffer.h"
#include "../logger/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>


int main(void) {

	int numRead, numParsed, numConsumed, i;

	// Instances of structure variables
	VN200_DEV gps;
	GPS_DATA data;

	printf("Initializing...\n");
	VN200GPSInit(&gps, 5);

	while(1) {

		numRead = VN200Poll(&gps);
		// printf("Read %d bytes from UART\n", numRead);

		do {

			numParsed = VN200GPSParse(&gps, &data);
			// printf("Parsed %d bytes from buffer\n", numParsed);

			if(numParsed > 0) {

				/*
				printf("\tData:\n");
				printf("\t\t");
				for(i = 0; i < numParsed; i++) {
					printf("%c", gps.inbuf.buffer[i]);
				}
				*/

				// Print out GPS data
				printf("\nGPS data:\n");
				printf("\tTime is %lf\n", data.time);
				printf("\t%hhd sats locked\n", data.NumSats);
				printf("\tLatitude is %lf\n", data.Latitude);
				printf("\tLongitude is %lf\n", data.Longitude);
				printf("\tAltitude is %lf\n\n", data.Altitude);

			}

			numConsumed = VN200Consume(&gps, numParsed);
			// printf("Consumed %d bytes from buffer\n", numConsumed);

		} while(numConsumed > 0);

		// usleep(100000);

	}

	VN200Destroy(&gps);

	return 0;

}

