/****************************************************************************\
 *
 * File:
 * 	logtest.c
 *
 * Description:
 * 	Tests the logger functionality
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/20/2019
 *
\***************************************************************************/

#include "logger.h"

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

	int i, rc;

	LOG_FILE testlog;
	char testval = 200;
	char data[DATA_LEN];

	rc = LogInit(&testlog, "logdirtest", "test", 1);
	if(rc) {
		perror("failed in main");
		return 1;
	}
	printf("Opened log file %s with fd %d\n", testlog.filename, testlog.fd);

	// Fill data array
	for(i = 0; i < DATA_LEN; i++) {
		data[i] = i;
	}

	printf("Writing to log...\n");
	LogUpdate(&testlog, data, DATA_LEN);

	printf("Closing log file...\n");
	LogClose(&testlog);

	printf("\n\nTest complete\n\n");

	// Success
	return 0;

} // main()

