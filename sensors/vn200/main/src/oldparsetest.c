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

    // Stores received packet data temporarily, immediately after parsing
    GPS_DATA gps_packet;
    IMU_DATA imu_packet;

    // Use logDebug(...) just like printf for printing stuff out
    logDebug("Initializing...\n");

    // Initialize VN200 device at 50Hz sample frequency, baud rate, for both IMU and GPS packets
    VN200Init(&dev, 50, VN200_BAUD, VN200_INIT_MODE_BOTH);

    // Force into both IMU and GPE concurrent output mode
    char *command = "VNWRG,06,248";
    VN200Command(&dev, command, strlen(command), 0);

    // Clear any received data from input buffer
    // (could instead verify confirmation message was received)
    VN200FlushOutput(&dev);

    // Loop forever (for test)
    while(1) {

        // Poll the device for any input data (distinct from linux service call poll (2))
        numRead = VN200Poll(&dev);
        logDebug("Read %d bytes from UART\n", numRead);

        // Loop until all input data has been parsed
        do { // while data left to parse

            // Store checksums
            unsigned char chkOld, chkNew;

            // Convenient reference to VN200 device input buffer
            unsigned char *packetBuf = dev.inbuf.buffer;

            // Determine start of first packet in buffer (starts on '$' character)
            int packetStart;
            for (packetStart = 0;
                    packetStart < dev.inbuf.length && dev.inbuf.buffer[packetStart] != '$'; packetStart++) {
                // Loop until $ found
            }

            // Determine end of first packet (data ends on '*' character, then has 2 character checksum)
            int chkStartIndex;
            for(chkStartIndex = packetStart; chkStartIndex < dev.inbuf.length - 3 && packetBuf[chkStartIndex] != '*'; chkStartIndex++) {
                // Loop until * found
            }

            // Read received checksum from input buffer
            int chkLength = sscanf(&(dev.inbuf.buffer[chkStartIndex + 1]), "%hhX", &chkOld);

            // Compute checksum of data (everything between $ and *, both excluded)
            chkNew = VN200CalculateChecksum(&(dev.inbuf.buffer[packetStart + 1]), chkStartIndex - packetStart - 1);

#if 0
            // Useful to keep around for debugging checksum, disable when not using
            logDebug("Read checksum from index %d (%c) to %d (%c)\n",
                    chkStartIndex + 1, packetBuf[chkStartIndex + 1],
                    chkStartIndex + chkLength + 1, packetBuf[chkStartIndex + chkLength + 1]);
            logDebug("Computed checksum from index %d (%c) to %d (%c)\n",
                    packetStart + 1, packetBuf[packetStart + 1],
                    chkStartIndex,
                    packetBuf[chkStartIndex]);
#endif


            // Verify checksums match
            if(chkNew != chkOld) {
                // Checksum failed, don't parse but skip to next packet
                logDebug("Checksum failed: Read %02X but computed %02X\n", chkOld, chkNew);

                // Signal to consume the packet anyway, it's garbage data
                numParsed = chkStartIndex + 2;

            } else {

                // Search for end of packet ID (ex. VNIMU), starts after first comma
                for(i = packetStart; i < chkStartIndex && packetBuf[i] != ','; i++) {
                    // Loop until comma is reached
                }

                // Length of packet ID string is the number travelled
                int packetIDLength = i - packetStart;

                // Ignore intro $VN*** ID, data starts after the comma
                int packetDataStart = i + 1;
                int packetLen = chkStartIndex - packetDataStart;

                /**** Determine type of packet and parse accordingly ****/

                // Packet is a GPS packet
                if(packetIDLength == 6 && 
                        !strncmp(&(packetBuf[packetStart]), "$VNGPE", packetIDLength)) {

                    /****************************** Note: This needs to be updated to parse VNGPE packets instead of VNGPS
                     ******************************/

                    // Print ID for debugging
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

                    // Eventually do something with timestamps
                    // gps_packet.timestamp = packet->timestamp;

                    // Check return code to ensure parsing succeeded, else remove packet anyway
                    if (rc > 0) {
                        numParsed = rc;

                        // Print data
                        logDebug("GPS packet data:\n"
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

                    // Parse as IMU packet
                    rc = VN200IMUPacketParse(&(packetBuf[packetDataStart]), packetLen, &imu_packet);

                    if (rc > 0) {
                        numParsed = rc;

                        // Print parsed data
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
                    } else {
                        numParsed = chkStartIndex + 2;
                    }

                } else {

                    // Packet is unknown type or improperly formatted
                    logDebug("Packet type unknown\n");

                } // Parsed packet type

            } // Checksum pass

            // If errors uncaught, remove some bytes anyway to move on to next packet
            if (numParsed <= 0) {
                numParsed = 3;
            }

            // Remove bytes from buffer
            numConsumed = VN200Consume(&dev, numParsed);
            logDebug("Consumed %d bytes (%d req) from buffer\n", numConsumed, numParsed);

        } while(numConsumed > 0);

    } // while(1)

    // Clean up device, for completeness
    VN200Destroy(&dev);

    return 0;

}

