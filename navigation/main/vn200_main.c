/***************************************************************************\
 *
 * File:
 * 	vn200_main.c
 *
 * Description:
 *	Tests the VN200 combined functionality (rolling back refactoring)
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/15/2020
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


int main(int argc, char **argv) {

    int i, rc;

    // Instances of structure variables
    VN200_DEV dev;

    // Stores received packet data temporarily, immediately after parsing
    GPS_DATA gps_packet;
    IMU_DATA imu_packet;

    // Use logDebug(L_DEBUG, ...) just like printf
    // Debug levels are L_INFO, L_DEBUG, L_VDEBUG
    logDebug(L_INFO, "Initializing...\n");

    char *devname;
    if (argc > 1) {
        devname = argv[1];
    } else {
        devname = VN200_DEVNAME;
    }

    // Initialize VN200 device at 50Hz sample frequency, baud rate, for both IMU and GPS packets
    VN200Init(&dev, devname, 50, VN200_BAUD, VN200_INIT_MODE_BOTH);

    // Force into both IMU and GPE concurrent output mode
    char *command = "VNWRG,06,248";
    VN200Command(&dev, command, strlen(command), 0);

    // Clear any received data from input buffer
    // (could instead verify confirmation message was received)
    VN200FlushOutput(&dev);

    // Loop forever (for test)
    while (1) {

        int numRead;

        // Number of bytes actually found
        int numParsed = 0, numConsumed = 0;

        // Poll the device for any input data (distinct from linux service call poll (2))
        numRead = VN200Poll(&dev);
        logDebug(L_DEBUG, "Read %d bytes from UART\n", numRead);

        // Loop until all input data has been parsed
        do { // while data left to parse

            // Store checksums
            unsigned char chkOld, chkNew;

            // Determine start of first packet in buffer (starts on '$' character)
            int packetStart;
            for (packetStart = 0;
                    packetStart < BufferLength(&(dev.inbuf)) && BufferIndex(&(dev.inbuf), packetStart) != '$'; packetStart++) {
                // Loop until $ found
            }

            // Determine end of first packet (data ends on '*' character, then has 2 character checksum)
            int chkStartIndex;
            for (chkStartIndex = packetStart; chkStartIndex < BufferLength(&(dev.inbuf)) - 3 && BufferIndex(&(dev.inbuf), chkStartIndex) != '*'; chkStartIndex++) {
                // Loop until * found
            }

            // Only parse the packet if there was a complete packet
            if (BufferIndex(&(dev.inbuf), chkStartIndex) == '*') {

                // Read received checksum from input buffer
                unsigned char chkReadData[2];
                BufferCopy(&(dev.inbuf), chkReadData, chkStartIndex + 1, 2);
                int chkLength = sscanf(chkReadData, "%hhX", &chkOld);

                // Compute checksum of data (everything between $ and *, both excluded)
                int chkComputeLength = chkStartIndex - packetStart - 1;
                {
                    unsigned char chkComputeData[chkComputeLength + 1];
                    BufferCopy(&(dev.inbuf), chkComputeData, packetStart + 1, chkComputeLength);
                    chkNew = VN200CalculateChecksum(chkComputeData, chkComputeLength);

                    // Useful to keep around for debugging checksum, disable when not using
                    logDebug(L_VVDEBUG, "Read checksum from index %d (%c) to %d (%c)\n",
                            chkStartIndex + 1, BufferIndex(&(dev.inbuf), chkStartIndex + 1),
                            chkStartIndex + chkLength + 1, BufferIndex(&(dev.inbuf), chkStartIndex + chkLength + 1));
                    logDebug(L_VVDEBUG, "Computed checksum from index %d (%c) to %d (%c)\n",
                            packetStart + 1, BufferIndex(&(dev.inbuf), packetStart + 1),
                            chkStartIndex,
                            BufferIndex(&(dev.inbuf), chkStartIndex));
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
                            && BufferIndex(&(dev.inbuf), i) != ',';
                            i++) {
                        // Loop until comma is reached, copy ID string into buffer
                        packetIDString[i - packetStart] = BufferIndex(&(dev.inbuf), i);
                    }

                    // Length of packet ID string is the number travelled
                    int packetIDLength = i - packetStart;

                    // Ignore intro $VN*** ID, data starts after the comma
                    int packetDataStart = i + 1;
                    int packetLen = chkStartIndex - packetDataStart;

                    /**** Determine type of packet and parse accordingly ****/

                    // Packet is a GPS packet
                    if (packetIDLength == 6 && !strncmp(packetIDString, "$VNGPE", packetIDLength)) {

                        /****************************** Note: This needs to be updated to parse VNGPE packets instead of VNGPS
                         ******************************/

                        unsigned char packetDataBuf[packetLen + 1];
                        BufferCopy(&(dev.inbuf), packetDataBuf, packetDataStart, packetLen);

                        // Print ID for debugging
                        logDebug(L_DEBUG, "Found a GPS Packet at index %d: %c%c%c%c%c%c\n",
                                packetStart,
                                BufferIndex(&(dev.inbuf), packetStart),
                                BufferIndex(&(dev.inbuf), packetStart+1),
                                BufferIndex(&(dev.inbuf), packetStart+2),
                                BufferIndex(&(dev.inbuf), packetStart+3),
                                BufferIndex(&(dev.inbuf), packetStart+4),
                                BufferIndex(&(dev.inbuf), packetStart+5));

                        // Parse as GPS packet
                        rc = VN200GPSPacketParse(packetDataBuf, packetLen, &gps_packet);

                        // Eventually do something with timestamps
                        // gps_packet.timestamp = packet->timestamp;

                        // Check return code to ensure parsing succeeded, else remove packet anyway
                        if (rc > 0) {
                            numParsed = rc;

                            // Print data
                            logDebug(L_DEBUG, "GPS packet data:\n"
                                    "\tLat: %f\n"
                                    "\tLon: %f\n"
                                    "\tAlt: %f\n",
                                    gps_packet.Latitude,
                                    gps_packet.Longitude,
                                    gps_packet.Altitude);
                        } else {
                            numParsed = chkStartIndex + 2;
                        }

                        // Packet is an IMU packet
                    } else if (packetIDLength == 6 && !strncmp(packetIDString, "$VNIMU", packetIDLength)) {

                        unsigned char packetDataBuf[packetLen + 1];
                        BufferCopy(&(dev.inbuf), packetDataBuf, packetDataStart, packetLen);

                        logDebug(L_DEBUG, "Found an IMU Packet at index %d: %c%c%c%c%c%c\n",
                                packetStart,
                                BufferIndex(&(dev.inbuf), packetStart),
                                BufferIndex(&(dev.inbuf), packetStart+1),
                                BufferIndex(&(dev.inbuf), packetStart+2),
                                BufferIndex(&(dev.inbuf), packetStart+3),
                                BufferIndex(&(dev.inbuf), packetStart+4),
                                BufferIndex(&(dev.inbuf), packetStart+5));

                        // Parse as IMU packet
                        rc = VN200IMUPacketParse(packetDataBuf, packetLen, &imu_packet);

                        if (rc > 0) {
                            numParsed = rc;

                            // Print parsed data
                            logDebug(L_DEBUG, "IMU packet data:\n"
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

                    } // Parsed packet type

                } // Checksum pass

                // If errors uncaught, remove some bytes anyway to move on to next packet
                if (numParsed <= 0) {
                    numParsed = 3;
                }

                // Remove bytes from buffer
                numConsumed = VN200Consume(&dev, numParsed);
                logDebug(L_DEBUG, "Consumed %d bytes (%d req) from buffer\n", numConsumed, numParsed);

            } else {
                numConsumed = 0;
            }

        } while (numConsumed > 0);

    } // while (1)

    // Clean up device, for completeness
    VN200Destroy(&dev);

    return 0;

}

