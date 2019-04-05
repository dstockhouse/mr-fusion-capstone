/****************************************************************************\
 *
 * File:
 * 	uarttest.c
 *
 * Description:
 * 	Tests the pingUSB UART functionality
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/13/2019
 *
\***************************************************************************/

#include "uart.h"
#include "../pingusb.h"
#include "../buffer/buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>


#define NUM_BYTES 204800

/**** Function main ****
 *
 * Test of the UART module functionality
 *
 * Arguments: None
 *
 * Return value:
 * 	Returns 0 on success
 * 	Returns a negative number on failure
 */
int main(void) {

	int i, rc, numRead, col = 0, loopCount = 0;
	int numBytes = 0;

	PINGUSB_RECV dev;

	rc = pingUSBInit(&dev);
	if(rc) {
		printf("Couldn't intialize device: %d\n", rc);
		return rc;
	}

	printf("Initializing receiver... %d\n", rc);
	printf("Entering polling loop (ctrl-c to exit)\n\n\t");

	while(numBytes < NUM_BYTES) {

		// printf("L: %d\n", loopCount);
		loopCount++;

		// Ping for new characters
		numRead = pingUSBPoll(&dev);
		// printf("Read %d chars\n", numRead);

		numBytes += numRead;

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
		// pingUSBConsume(&dev, dev.inbuf.length);

		BufferRemove(&(dev.inbuf), dev.inbuf.length);

		usleep(10000);
	}

	printf("\n\nTest complete\n\n");

	pingUSBDestroy(&dev);

	// Success
	return 0;

} // main()

