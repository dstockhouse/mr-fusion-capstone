/***************************************************************************\
 *
 * File:
 * 	oldparsetest.c
 *
 * Description:
 *	Tests the VN200 combined functionality (rolling back refactoring)
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/12/2020
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "utils.h"
#include "buffer.h"
#include "logger.h"
#include "debuglog.h"
#include "uart.h"

#include "VN200.h"
#include "VN200_CRC.h"
#include "VN200_GPS.h"
#include "VN200_IMU.h"


int main(void) {

    int numRead, numParsed, numConsumed, i, rc;

    // Instances of structure variables
    VN200_DEV dev;
    GPS_DATA gps_packet;
    IMU_DATA imu_packet;

    logDebug("Initializing...\n");
    VN200Init(&dev, 50, VN200_BAUD, VN200_INIT_MODE_BOTH);

    char *command = "VNWRG,06,248";
    VN200Command(&dev, command, strlen(command), 0);
    VN200FlushOutput(&dev);

    while(1) {

        numRead = VN200Poll(&dev);
        logDebug("Read %d bytes from UART\n", numRead);

        do {

            unsigned char chkOld, chkNew;

            unsigned char *packetBuf = dev.inbuf.buffer;

            int packetStart;
            for (packetStart = 0;
                    packetStart < dev.inbuf.length && dev.inbuf.buffer[packetStart] != '$'; packetStart++) {
                // Loop until $ found
            }

            int chkStartIndex;
            for(chkStartIndex = packetStart; chkStartIndex < dev.inbuf.length - 3 && packetBuf[chkStartIndex] != '*'; chkStartIndex++) {
                // Loop until asterisk is reached
            }

            int chkLength = sscanf(&(dev.inbuf.buffer[chkStartIndex + 1]), "%hhX", &chkOld);
            chkNew = VN200CalculateChecksum(&(dev.inbuf.buffer[packetStart + 1]), chkStartIndex - packetStart - 1);

#if 0
    logDebug("Read checksum from index %d (%c) to %d (%c)\n",
            chkStartIndex + 1, packetBuf[chkStartIndex + 1],
            chkStartIndex + chkLength + 1, packetBuf[chkStartIndex + chkLength + 1]);
    logDebug("Computed checksum from index %d (%c) to %d (%c)\n",
            packetStart + 1, packetBuf[packetStart + 1],
            chkStartIndex,
            packetBuf[chkStartIndex]);
#endif


            if(chkNew != chkOld) {
                // Checksum failed, don't parse but skip to next packet
                logDebug("Checksum failed: Read %02X but computed %02X\n", chkOld, chkNew);
                numParsed = packetStart + 3;
            } else {

                // Search for end of packet ID (ex. VNIMU)
                for(i = packetStart; i < chkStartIndex && packetBuf[i] != ','; i++) {
                    // Loop until comma is reached
                }

                // Length of packet ID string is the number travelled
                int packetIDLength = i - packetStart;

                // Ignore intro $VN***
                int packetDataStart = i + 1;
                int packetLen = chkStartIndex - packetDataStart;

                /**** Determine type of packet and parse accordingly ****/

                // Packet is a GPS packet
                if(packetIDLength == 6 && 
                        !strncmp(&(packetBuf[packetStart]), "$VNGPE", packetIDLength)) {

                    logDebug("Found a GPS Packet at index %d: %c%c%c%c%c%c\n",
                            packetStart,
                            packetBuf[packetStart],
                            packetBuf[packetStart+1],
                            packetBuf[packetStart+2],
                            packetBuf[packetStart+3],
                            packetBuf[packetStart+4],
                            packetBuf[packetStart+5]);

                    // Parse as GPS packet
                    rc = VN200GPSPacketParse(&(packetBuf[packetDataStart]), packetLen, &gps_packet);
                    // gps_packet.timestamp = packet->timestamp;

                    logDebug("GPS packet data:\n"
                            "\tLat: %f\n"
                            "\tLon: %f\n"
                            "\tAlt: %f\n",
                            gps_packet.Latitude,
                            gps_packet.Longitude,
                            gps_packet.Altitude);

                    if (rc > 0) {
                        numParsed = rc;
                    } else {
                        numParsed = packetStart;
                    }

                    // Packet is an IMU packet
                } else if(packetIDLength == 6 && 
                        !strncmp(&(packetBuf[packetStart]), "$VNIMU", packetIDLength)) {

                    logDebug("Found an IMU Packet at index %d: %c%c%c%c%c%c\n",
                            packetStart,
                            packetBuf[packetStart],
                            packetBuf[packetStart+1],
                            packetBuf[packetStart+2],
                            packetBuf[packetStart+3],
                            packetBuf[packetStart+4],
                            packetBuf[packetStart+5]);

                    logDebug("IMU packet data:\n"
                            "\tAccel: %f, %f, %f\n"
                            "\tGyro: %f, %f, %f\n"
                            "\tCompass: %f, %f, %f\n",
                            imu_packet.accel[0],
                            imu_packet.accel[1],
                            imu_packet.accel[2],
                            imu_packet.gyro[0],
                            imu_packet.gyro[1],
                            imu_packet.gyro[2],
                            imu_packet.compass[0],
                            imu_packet.compass[1],
                            imu_packet.compass[2]);

                    // Parse as IMU packet
                    rc = VN200IMUPacketParse(&(packetBuf[packetDataStart]), packetLen, &imu_packet);

                    if (rc > 0) {
                        numParsed = rc;
                    } else {
                        numParsed = packetStart;
                    }

                } else {

                    // Packet is unknown type or improperly formatted
                    logDebug("Packet type unknown\n");

                }

#if 0
                numParsed = VN200GPSPacketParse(&gps, &data);
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
#endif
            } // Checksum pass

            if (numParsed <= 0) {
                numParsed = 3;
            }

            numConsumed = VN200Consume(&dev, numParsed);
            logDebug("Consumed %d bytes (%d req) from buffer\n", numConsumed, numParsed);

        } while(numConsumed > 0);

        // usleep(100000);

    }

    VN200Destroy(&dev);

    return 0;

}

