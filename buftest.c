/****************************************************************************\
 *
 * File:
 * 	buftest.c
 *
 * Description:
 * 	Tests the buffer functionality
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/13/2019
 *
\***************************************************************************/

#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#define DATA_LEN 20000
#define REMOVE_LEN 3000


/**** Function main ****
 *
 * Test of the buffer module functionality
 *
 * Arguments: None
 *
 * Return value:
 * 	Returns 0 on success
 * 	Returns a negative number on failure
 */
int main(void) {

	int i;

	BYTE_BUFFER testbuf;
	unsigned char testval = 200;
	unsigned char data[DATA_LEN];

	// Fill data array
	for(i = 0; i < DATA_LEN; i++) {
		data[i] = i;
	}

	// Initialize
	BufferEmpty(&testbuf);

	printf("Adding %d...\n", testval);

	BufferAdd(&testbuf, testval);

	printf("\tNew size: %d\n\t", testbuf.length);

	for(i = 0; i < testbuf.length; i++) {
		printf("%d, ", testbuf.buffer[i]);
		if(i % 20 == 19) printf("\n\t");
	}

	printf("\n\nAdding array...\n");

	BufferAddArray(&testbuf, data, DATA_LEN);

	printf("\tNew size: %d\n\t", testbuf.length);

	for(i = 0; i < testbuf.length; i++) {
		printf("%d, ", testbuf.buffer[i]);
		if(i % 20 == 19) printf("\n\t");
	}

	printf("\n\nRemoving %d...\n", REMOVE_LEN);

	BufferRemove(&testbuf, REMOVE_LEN);

	printf("\tNew size: %d\n\t", testbuf.length);

	for(i = 0; i < testbuf.length; i++) {
		printf("%d, ", testbuf.buffer[i]);
		if(i % 20 == 19) printf("\n\t");
	}

	printf("\n\nRemoving too many (%d)...\n", testbuf.length + 30);

	i = BufferRemove(&testbuf, testbuf.length);

	printf("\tNew size: %d\n\t", testbuf.length);

	for(i = 0; i < testbuf.length; i++) {
		printf("%d, ", testbuf.buffer[i]);
		if(i % 20 == 19) printf("\n\t");
	}

	printf("\n\nTest complete\n\n");

	// Success
	return 0;

} // main()

