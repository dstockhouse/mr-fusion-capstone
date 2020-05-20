/****************************************************************************
 *
 * File:
 *      vn200_run.c
 *
 * Description:
 *      Implementation of the vn200 hardware interface control flow
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 04/23/2020
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "config.h"
#include "utils.h"
#include "buffer.h"
#include "logger.h"
#include "uart.h"
#include "thread.h"

#include "vn200.h"
#include "vn200_crc.h"
#include "vn200_gps.h"
#include "vn200_imu.h"

#include "navigation.h"

// Declared as extern in vn200_run.h
GPS_DATA gps_packet;
IMU_DATA imu_packet;
int vn200_continueRunning = 1;

int vn200_run(NAVIGATION_PARAMS *navigation) {

    int i, rc;

    // Local pointer to VN200 device
    VN200_DEV *dev = &(navigation->vn200);

    logDebug(L_INFO, "  Starting vn200 thread\n");

    // Loop until stopped by main thread
    while (vn200_continueRunning) {

        // Read/parse counters
        int numRead, numParsed = 0, numConsumed = 0;

        // Poll the device for any input data (non-blocking, unlike linux service call poll (2))
        numRead = VN200Poll(dev);
        logDebug(L_VDEBUG, "Read %d bytes from UART\n", numRead);

        // Loop until all input data has been parsed
        do { // while data left to parse

            // Store checksums
            unsigned char chkOld, chkNew;

            // Determine start of first packet in buffer (starts on '$' character)
            int packetStart;
            for (packetStart = 0;
                    packetStart < BufferLength(&(dev->inbuf)) && BufferIndex(&(dev->inbuf), packetStart) != '$'; packetStart++) {
                // Loop until $ found
            }

            // Determine end of first packet (data ends on '*' character, then has 2 character checksum)
            int chkStartIndex;
            for (chkStartIndex = packetStart; chkStartIndex < BufferLength(&(dev->inbuf)) - 3 && BufferIndex(&(dev->inbuf), chkStartIndex) != '*'; chkStartIndex++) {
                // Loop until * found
            }

            // Only parse the packet if there was a complete packet
            if (BufferIndex(&(dev->inbuf), chkStartIndex) == '*') {

                // Read received checksum from input buffer
                unsigned char chkReadData[2];
                BufferCopy(&(dev->inbuf), chkReadData, chkStartIndex + 1, 2);
                int chkLength = sscanf((char *) chkReadData, "%hhX", &chkOld);

                // Compute checksum of data (everything between $ and *, both excluded)
                int chkComputeLength = chkStartIndex - packetStart - 1;
                {
                    unsigned char chkComputeData[chkComputeLength + 1];
                    BufferCopy(&(dev->inbuf), chkComputeData, packetStart + 1, chkComputeLength);
                    chkNew = VN200CalculateChecksum(chkComputeData, chkComputeLength);

                    // Useful to keep around for debugging checksum, disable when not using
                    logDebug(L_VVDEBUG, "Read checksum from index %d (%c) to %d (%c)\n",
                            chkStartIndex + 1, BufferIndex(&(dev->inbuf), chkStartIndex + 1),
                            chkStartIndex + chkLength + 1, BufferIndex(&(dev->inbuf), chkStartIndex + chkLength + 1));
                    logDebug(L_VVDEBUG, "Computed checksum from index %d (%c) to %d (%c)\n",
                            packetStart + 1, BufferIndex(&(dev->inbuf), packetStart + 1),
                            chkStartIndex,
                            BufferIndex(&(dev->inbuf), chkStartIndex));
                    logDebug(L_VVDEBUG, "  Full packet data:\n    ");
                    int i;
                    for (i = 0; i < chkComputeLength; i++) {
                        logDebug(L_VVDEBUG, "%c", (char) chkComputeData[i]);
                    }
                    logDebug(L_VVDEBUG, "\n\n");

                } // Temporary block for computing checksum on stack


                // Verify checksums match
                if (chkNew != chkOld) {
                    // Checksum failed, don't parse but skip to next packet
                    logDebug(L_INFO, "Checksum failed: Read %02X but computed %02X\n", chkOld, chkNew);

                    // Signal to consume the packet anyway, it's garbage data
                    numParsed = chkStartIndex + 2;

                } else {

                    // Search for end of packet ID (ex. VNIMU), starts after first comma
                    const int packetIDMaxLength = 16;
                    unsigned char packetIDString[packetIDMaxLength];
                    for (i = packetStart;
                            i < chkStartIndex
                            && i - packetStart < packetIDMaxLength
                            && BufferIndex(&(dev->inbuf), i) != ',';
                            i++) {
                        // Loop until comma is reached, copy ID string into buffer
                        packetIDString[i - packetStart] = BufferIndex(&(dev->inbuf), i);
                    }

                    // Length of packet ID string is the number travelled
                    int packetIDLength = i - packetStart;

                    // Ignore intro $VN*** ID, data starts after the comma
                    int packetDataStart = i + 1;
                    int packetLen = chkStartIndex - packetDataStart;

                    /**** Determine type of packet and parse accordingly ****/

                    // Packet is a GPS packet
                    if (packetIDLength == 6 && !strncmp((char *) packetIDString, "$VNGPE", packetIDLength)) {

                        /****************************** Note: This needs to be updated to parse VNGPE packets instead of VNGPS
                         ******************************/

                        unsigned char packetDataBuf[packetLen + 1];
                        BufferCopy(&(dev->inbuf), packetDataBuf, packetDataStart, packetLen);

                        // Print ID for debugging
                        logDebug(L_VDEBUG, "Found a GPS Packet at index %d: %c%c%c%c%c%c\n",
                                packetStart,
                                BufferIndex(&(dev->inbuf), packetStart),
                                BufferIndex(&(dev->inbuf), packetStart+1),
                                BufferIndex(&(dev->inbuf), packetStart+2),
                                BufferIndex(&(dev->inbuf), packetStart+3),
                                BufferIndex(&(dev->inbuf), packetStart+4),
                                BufferIndex(&(dev->inbuf), packetStart+5));

                        // Parse as GPS packet
                        rc = VN200GPSPacketParse(packetDataBuf, packetLen, &gps_packet);

                        // Check return code to ensure parsing succeeded, else remove packet anyway
                        if (rc > 0) {

                            numParsed = rc;

                            // Adjust timestamp to count from the beginning of the program
                            gps_packet.timestamp -= navigation->startTime;

                            // Log the parsed data to a file
                            VN200GPSLogParsed(&(dev->logFileGPSParsed), &gps_packet);

                            // Print data
                            logDebug(L_VDEBUG, "GPS packet data:\n"
                                    "\tX: %f\n"
                                    "\tY: %f\n"
                                    "\tZ: %f\n",
                                    gps_packet.PosX,
                                    gps_packet.PosY,
                                    gps_packet.PosZ);
                        } else {
                            numParsed = chkStartIndex + 2;
                        }

                        // Packet is an IMU packet
                    } else if (packetIDLength == 6 && !strncmp((char *) packetIDString, "$VNIMU", packetIDLength)) {

                        unsigned char packetDataBuf[packetLen + 1];
                        BufferCopy(&(dev->inbuf), packetDataBuf, packetDataStart, packetLen);

                        logDebug(L_VDEBUG, "Found an IMU Packet at index %d: %c%c%c%c%c%c\n",
                                packetStart,
                                BufferIndex(&(dev->inbuf), packetStart),
                                BufferIndex(&(dev->inbuf), packetStart+1),
                                BufferIndex(&(dev->inbuf), packetStart+2),
                                BufferIndex(&(dev->inbuf), packetStart+3),
                                BufferIndex(&(dev->inbuf), packetStart+4),
                                BufferIndex(&(dev->inbuf), packetStart+5));

                        // Parse as IMU packet
                        rc = VN200IMUPacketParse(packetDataBuf, packetLen, &imu_packet);

                        if (rc > 0) {

                            numParsed = rc;

                            // Adjust timestamp to count from the beginning of the program
                            imu_packet.timestamp -= navigation->startTime;

                            // Log the parsed data to a file
                            VN200IMULogParsed(&(dev->logFileIMUParsed), &imu_packet);

                            // Print parsed data
                            logDebug(L_VDEBUG, "IMU packet data:\n"
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
                        } else {
                            numParsed = chkStartIndex + 2;
                        }

                    } else {

                        // Packet is unknown type or improperly formatted
                        logDebug(L_INFO, "Packet type unknown\n");
                        numParsed = chkStartIndex + 2;

                    } // Parsed packet type

                } // Checksum pass

                // If errors uncaught, remove some bytes anyway to move on to next packet
                if (numParsed <= 0) {
                    numParsed = 3;
                }

                // Remove bytes from buffer
                numConsumed = VN200Consume(dev, numParsed);
                logDebug(L_VDEBUG, "Consumed %d bytes (%d req) from buffer\n", numConsumed, numParsed);

            } else {
                numConsumed = 0;
            }

        } while (numConsumed > 0);

    } // while (vn200_continueRunning)

    printf("\n\n  Ending VN200 thread\n");

	return 0;
}

