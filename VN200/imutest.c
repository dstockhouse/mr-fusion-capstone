/***************************************************************************\
 *
 * File:
 * 	imutest.c
 *
 * Description:
 *	Tests the VN200 IMU functionality
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/07/2019
 *
 ***************************************************************************/

#include "VN200.h"
#include "VN200_IMU.h"

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
	VN200_DEV imu;
	IMU_DATA data;

	printf("Initializing...\n");
	VN200IMUInit(&imu, 1);

	while(1) {

		numRead = VN200Poll(&imu);
		printf("Read %d bytes from UART\n", numRead);

		if(numRead > 0) {
			do {

				numParsed = VN200IMUParse(&imu, &data);
				printf("Parsed %d bytes from buffer\n", numParsed);

				printf("\tData:\n");
				printf("\t\t");
				for(i = 0; i < numParsed; i++) {
					printf("%c", imu.inbuf.buffer[i]);
				}

				numConsumed = VN200Consume(&imu, numParsed);
				printf("Consumed %d bytes from buffer\n", numConsumed);

				// Print out IMU data
				printf("\nIMU data:\n");

				printf("\tCompass: %lf, %lf, %lf\n",
						data.compass[0], data.compass[1], data.compass[2]);

				printf("\tAccel: %lf, %lf, %lf\n",
						data.accel[0], data.accel[1], data.accel[2]);

				printf("\tGyro: %lf, %lf, %lf\n",
						data.gyro[0], data.gyro[1], data.gyro[2]);

				printf("\tTemp: %lf\n", data.temp);
				printf("\tBaro: %lf\n", data.baro);

			} while(numConsumed > 0);
		}

		usleep(100000);

	}

	VN200Destroy(&imu);

	return 0;

}

