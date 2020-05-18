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


// Stores received packet data temporarily, immediately after parsing
// Also used to share data between threads (unsafe but temporary behavior)
GPS_DATA gps_packet;
IMU_DATA imu_packet;

int vn200Continue = 1;


void *vn200test_run(void *param) {

    int i, rc;

    // Input parameter is the initialized kangaroo device
    VN200_DEV *dev = (VN200_DEV *) param;

    logDebug(L_INFO, "  Starting vn200 thread\n");

    while (vn200Continue) {

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

                        // Eventually do something with timestamps
                        // gps_packet.timestamp = packet->timestamp;

                        // Check return code to ensure parsing succeeded, else remove packet anyway
                        if (rc > 0) {
                            numParsed = rc;

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

    } // while (vn200Continue)

    printf("  Ending VN20 thread\n");

    return (void *) 0;
}

int main(int argc, char **argv) {

    int i, rc;

    // Instances of structure variables
    VN200_DEV dev;

    // Use logDebug(L_DEBUG, ...) just like printf
    // Debug levels are L_INFO, L_DEBUG, L_VDEBUG
    logDebug(L_INFO, "Initializing...\n");

    // Set nonblocking input mode for user control
    setStdinNoncanonical(1);

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

    // Dispatch reader thread at highest priority
    pthread_attr_t vn200Attr;
    pthread_t vn200Thread;

    rc = ThreadAttrInit(&vn200Attr, 0);
    if (rc < 0) {
        logDebug(L_INFO, "Failed to set thread attributes: %s\n",
                strerror(errno));
        return 1;
    }
    rc = ThreadCreate(&vn200Thread, &vn200Attr, &vn200test_run, (void *) &dev);
    if (rc < 0) {
        logDebug(L_INFO, "Failed to start VN200 thread: %s\n",
                strerror(errno));
        return 1;
    }

    usleep(100000);

    printf("Press any key to end\n\n");

    // Loop Until ending
    int loopContinue = 1;
    while (loopContinue) {

        // Get input from user nonblocking
        char input[16];
        rc = read(STDIN_FILENO, input, 16);

        if (rc > 0) {
            loopContinue = 0;
        }

        printf("  fx: %.3f  fy: %.3f  fz: %.3f\n",
                imu_packet.accel[0], imu_packet.accel[1], imu_packet.accel[2]);

        printf("  X: %.6f  Y: %.6f  Z: %.6f\r\033[A",
                gps_packet.PosX, gps_packet.PosY, gps_packet.PosZ);

    } // while (loopContinue)

    vn200Continue = 0;
    usleep(100000);

    printf("\n\nAttempting to join reader thread...\n");
    do {

        rc = ThreadTryJoin(vn200Thread, NULL);
        usleep(100000);

    } while (rc != 0 && errno == EBUSY);

    // Clean up device, for completeness
    VN200Destroy(&dev);

    // Restore terminal mode
    setStdinNoncanonical(0);

    return 0;

}

