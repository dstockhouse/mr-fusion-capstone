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
#include <string.h>
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

            numParsed = VN200Parse(&dev);
            logDebug(L_DEBUG, "Parsed %d bytes from buffer\n", numParsed);

            logDebug(L_DEBUG, "packet ring buffer not empty: start = %d, end = %d\n", dev.ringbuf.start, dev.ringbuf.end);

            int packetIndex;
            for (packetIndex = dev.ringbuf.start;
                    packetIndex != dev.ringbuf.end;
                    packetIndex = (packetIndex + 1) % VN200_PACKET_RING_BUFFER_SIZE) {


                VN200_PACKET *packet = &(dev.ringbuf.packets[dev.ringbuf.start]);

                logDebug("\tLooping packets: i=%d, pi=%d\n", packetIndex, packet->startIndex);

                if (packet->contentsType == VN200_PACKET_CONTENTS_TYPE_GPS) {
                    GPS_DATA *gps_packet = &(packet->GPSData);
                    logDebug("GPS packet data:\n"
                            "\tLat: %f\n"
                            "\tLon: %f\n"
                            "\tAlt: %f\n",
                            gps_packet->Latitude,
                            gps_packet->Longitude,
                            gps_packet->Altitude);

                } else if (packet->contentsType == VN200_PACKET_CONTENTS_TYPE_IMU) {
                    IMU_DATA *imu_packet = &(packet->IMUData);
                    logDebug("IMU packet data:\n"
                            "\tAccel: %f, %f, %f\n"
                            "\tGyro: %f, %f, %f\n"
                            "\tCompass: %f, %f, %f\n",
                            imu_packet->accel[0],
                            imu_packet->accel[1],
                            imu_packet->accel[2],
                            imu_packet->gyro[0],
                            imu_packet->gyro[1],
                            imu_packet->gyro[2],
                            imu_packet->compass[0],
                            imu_packet->compass[1],
                            imu_packet->compass[2]);

                } else {
                    logDebug("Packet unidentified.\n");
                }
            }

            logDebug("There are %d packets still in ring buffer.\n",
                    VN200_PACKET_RING_BUFFER_MOD(dev.ringbuf.end - dev.ringbuf.start));

            numConsumed = VN200Consume(&dev, numParsed);

            logDebug("%d bytes left in input buffer.\n", dev.inbuf.length);

        } while (numConsumed > 0);
#endif

        numConsumed = VN200Consume(&dev, numRead);
        logDebug(L_DEBUG, "Consumed %d bytes from buffer\n", numConsumed);

        // usleep(10);

    }

    VN200Destroy(&dev);

    return 0;

}

