/***************************************************************************\
 *
 * File:
 * 	vn200rawsimpletest.c
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
#include <signal.h>
#include <sys/types.h>

#include "control.h"
#include "debuglog.h"
#include "logger.h"
#include "uart.h"
#include "VN200.h"


int main(void) {

	int numRead, numParsed, numConsumed, i;
	double time;

	/*
	printf("Forking child\n");

	pid_t forkReturn = fork();
	if(forkReturn == 0) {
		printf("Child running puttyLog.sh\n");
		system("/home/pi/icarus-dronenet/script/puttyLog.sh 57600");
		printf("Child exiting\n");
		return 0;
	} else {
		sleep(5);
		printf("Killing child\n");
		system("killall plink");
		system("killall plink");
		system("killall plink");
		kill(forkReturn, SIGINT);
	}

	printf("Resuming execution\n");
	*/

	LOG_FILE log;

	// Initialize log
	LogInit(&log, "log/SampleData/raw", "VN200", LOG_FILEEXT_LOG);

	const int buflen = 16384;
	char buf[buflen];
	char *command;

	int fd = UARTInit(VN200_DEVNAME, 115200);
	if(fd < 0) {
		printf("Failed to open UART device\n");
		return 1;
	}

/*
	// Change VN200 baud to 115200
	char *command = "$VNWRG,05,115200*XX\n\r";
	logDebug(command);
	UARTWrite(fd, command, strlen(command));
	sleep(1);
	UARTRead(fd, buf, buflen);

	// Change CPU baud
	logDebug("Resetting UART Baud to 115200\n");
	UARTSetBaud(fd, 115200);
	sleep(1);
	UARTRead(fd, buf, buflen);

*/

	// Configure

	command = "$VNWRG,06,00*XX\n\r";
	logDebug(command);
	UARTWrite(fd, command, strlen(command));
	sleep(1);
	UARTRead(fd, buf, buflen);

	command = "$VNWRG,06,19*XX\n\r";
	logDebug(command);
	UARTWrite(fd, command, strlen(command));
	sleep(1);
	UARTRead(fd, buf, buflen);

	command = "$VNWRG,07,50*XX\n\r";
	logDebug(command);
	UARTWrite(fd, command, strlen(command));
	sleep(1);
	UARTRead(fd, buf, buflen);

	command = "$VNWRG,06,00*XX\n\r";
	logDebug(command);
	UARTWrite(fd, command, strlen(command));
	sleep(1);
	UARTRead(fd, buf, buflen);

	command = "$VNWRG,06,248*XX\n\r";
	logDebug(command);
	UARTWrite(fd, command, strlen(command));
	sleep(1);
	UARTRead(fd, buf, buflen);

	logDebug("Closing...");
	UARTClose(fd);
	sleep(3);

	logDebug("Reopening...");
	fd = UARTInitReadOnly(VN200_DEVNAME, 115200);
	if(fd < 0) {
		printf("Failed to open UART device\n");
		return 1;
	}


	int timeout = 0;

	int cumulative = 0;
	while(1) {
	// while(cumulative < 100000 && timeout < 1000) {

		getTimestamp(NULL, &time);
		logDebug("\nCurrent Time: %lf\n", time);

		numRead = UARTRead(fd, buf, buflen);
		cumulative += numRead;
		logDebug("Read %d bytes from UART\n", numRead);

		if(numRead > 0) {

			if(numRead < buflen) {
				buf[numRead] = '\0';
			} else {
				buf[numRead - 1] = '\0';
			}

			logDebug("\tRaw Data:\n");
			logDebug("%s", buf);

			int numWritten = LogUpdate(&log, buf, numRead);
			if(numWritten < numRead) {
				logDebug("ERRROR: Read %d but wrote %d \n", numRead, numWritten);
			}

			timeout = 0;

		} else {
			timeout++;
		}

		// Flush file I/O
		close(log.fd);
		log.fd = open(log.filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
	}

	UARTClose(fd);

	return 0;

}

