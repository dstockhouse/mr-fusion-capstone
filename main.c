/****************************************************************************\
 *
 * File:
 * 	main.c
 *
 * Description:
 * 	Tests the functionality of the combined parser and UART interface.
 * 	This code uses realtime threads to allow more predictable timing
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/25/2019
 *
 * Revision 0.2
 * 	Last edited 2/28/2019
 *
 * Revision 0.3
 * 	Last edited 4/05/2019
 * 	Started realtime functionality
 *
\***************************************************************************/

#include "buffer/buffer.h"
#include "uart/uart.h"
#include "pingusb.h"
#include "adsb_parser/adsb_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <sys/types.h>

// Multithreading includes
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>

/**** Global variables (shared by all threads) ****/

// USB device object
PINGUSB_RECV dev;
// Mutex lock around usb device object
pthread_mutex_t pingusbMutex;
pthread_mutexattr_t pingusbMutexAttr;


#define NUM_BYTES 2048

/**** Function main ****
 *
 * Test of the ADS_B receiver functionality
 *
 * Arguments: None
 *
 * Return value:
 * 	Returns 0 on success, anything else on failure
 */
int main(int argc, char **argv) {

	int i, rc, numRead, col = 0, loopCount = 0;
	int numBytes = 0;


	// Process command-line options
	if(argc == 1) {

		// Assume UART operation
		printf("Initializing receiver... \n");
		rc = pingUSBInit(&dev);
		if(rc) {
			printf("Couldn't intialize device: %d\n", rc);
			return rc;
		}
	} else {
		// Read from list of input files
		// Unimplemented
		printf("Only configured to read from %s. Exiting\n", PINGUSB_RECV_DEV);
		return -1;
	}


	/**** Start realtime multithreading configuration ****/

	// Mutex lock init
	rc = pthread_mutexattr_init(&pingusbMutexAttr);
	if(rc) {
		printf("Mutex attr init failed\n");
		return -2;
	}
	rc = pthread_mutex_init(&pingusbMutex, &pingusbMutexAttr);
	if(rc) {
		printf("Mutex init failed\n");
		return -2;
	}






	printf("Entering polling loop to collect %d bytes (ctrl-c to exit)\n\n\t", NUM_BYTES);

	while(numBytes < NUM_BYTES) {

		// printf("L: %d\n", loopCount);
		loopCount++;

		// Poll for new characters from UART
		numRead = pingUSBPoll(&dev);
		// printf("Read %d chars\n", numRead);

		numBytes += numRead;


		/* Uncomment to print out hex bytes as they are read *

		// Print chars received in hex
		for(i = 0; i < dev.inbuf.length; i++) {

			// Search for packet start
			if(dev.inbuf.buffer[i] == 0xfe) {
				printf("\n\n\t");
				col = 0;
			}

			printf("%02x", dev.inbuf.buffer[i]);
			col++;
			if(col % 2 == 0) {
				printf(" ");
			}
			if(col % 8 == 0) {
				printf(" ");
			}
			if(col % 16 == 0) {
				printf("\n\t");
			}
		}
		*/


		// Remove elements from input FIFO
		// pingUSBConsume(&dev, dev.inbuf.length);
		// BufferRemove(&(dev.inbuf), dev.inbuf.length);

		// Parse all data in USB receiver
		printf("Parsing data...\n");
		while(pingUSBParse(&dev) > 0) {

			printData(&(dev.packetData));

		}

		// usleep(1000000);
	}

	printf("\n\nTest complete\n\n");

	pingUSBDestroy(&dev);

	// Success
	return 0;

} // main()

