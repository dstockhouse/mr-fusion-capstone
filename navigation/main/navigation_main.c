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
 * 	Last edited 02/15/2020
 *
 * Revision 0.2
 * 	Last edited 05/18/2020
 * 	Integrated with separate thread(s) for vn200 interfacing
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <poll.h>

#include "config.h"
#include "utils.h"
#include "buffer.h"
#include "logger.h"
#include "uart.h"
#include "thread.h"
#include "tcp.h"

#include "vn200.h"
#include "vn200_crc.h"
#include "vn200_gps.h"
#include "vn200_imu.h"

#include "navigation.h"
#include "navigation_run.h"
#include "vn200_run.h"

int main(int argc, char **argv) {

    int interactiveMode = 0;
    int i, rc;

    // Instance of subsystem control structure
    NAVIGATION_PARAMS navigation;

    // Use logDebug(L_DEBUG, ...) just like printf
    // Debug levels are L_INFO, L_DEBUG, L_VDEBUG
    logDebug(L_INFO, "Initializing...\n");

    char *devname;
    if (argc > 1) {
        devname = argv[1];
    } else {
        devname = VN200_DEVNAME;
    }

    // Pause to allow spawning process a chance to start server
    sleep(1);

    // Initialize sockets
    // This deployment only initialize in non-interactive mode and only guidance
    navigation.guidance_sock = -1;
    navigation.control_sock = -1;
    navigation.imageproc_sock = -1;

    // See skeleton_navigation_main for proper and complete tcp initialization
    logDebug(L_DEBUG, "Navigation: Attempting to connect to guidance at %s:%d\n", GUIDANCE_IP_ADDR, NAVIGATION_TCP_PORT);
    int gSock = TCPClientInit();
    if (gSock == -1) {
        logDebug(L_INFO, "Navigation: Failed to initialize navigation sockets: %s\n", strerror(errno));
    }

    // Loop until all sockets are connected
    const int MAX_CONNECT_ATTEMPTS = 10000;
    int gConnected = 0, numTries = 0; 
    while (!gConnected && numTries < MAX_CONNECT_ATTEMPTS) {

        // Count attempt number
        numTries++;

        // Attempt to connect to guidance (if not already connected)
        if (!gConnected) {

            rc = TCPClientTryConnect(gSock, GUIDANCE_IP_ADDR, NAVIGATION_TCP_PORT);
            if (rc != -1) {
                logDebug(L_INFO, "Navigation: Successful TCP connection to guidance\n");
                gConnected = 1;
                navigation.guidance_sock = gSock;
            } else if (errno == ECONNREFUSED) {
                logDebug(L_DEBUG, "Navigation: Unsuccessful connection to guidance, will try again\n");
                // Delay to give other end a chance to start
                usleep(100000);
            } else {
                logDebug(L_INFO, "Navigation: Could not connect to guidance: %s\n", strerror(errno));
            }
        }

    } // while (!connected && tries remaining)

    // Too many attempts to establish connection
    if (numTries >= MAX_CONNECT_ATTEMPTS) {

        logDebug(L_INFO, "Navigation: Exceeded maximum number of TCP connection attempts (%d)\n", MAX_CONNECT_ATTEMPTS);

        TCPClose(gSock);

        // Since we couldn't connect, assume user has terminal
        interactiveMode = 1;
    }

    // Get configuration ID's for this run (start time and numeric key)
    if (interactiveMode) {

        // Set nonblocking input mode for user control
        setStdinNoncanonical(1);

        // Zero time is now
        getTimestamp(NULL, &(navigation.startTime));

        // Get random key for logging
        srand((unsigned) round(navigation.startTime));
        printf("Seeding RNG with %d\n", (unsigned) round(navigation.startTime));
        navigation.key = rand();

    } else {

        // Wait for initialization message from guidance
        logDebug(L_INFO, "Waiting for initialization message from guidance...\n");

        // The message from guidance has the following form
        //      init[startTime][key]
        // where
        //  0:  'init' is a literal 4-byte start of message,
        //  4:  startTime is a 8-byte double, the time to start
        // 12:  key is a 4-byte unsigned int that is the key for logging purposes

        unsigned char tcpBuf[512] = {0};
        int numReceived = 0, messageReceived = 0;

        const double MAX_TCP_WAIT = 30.0;
        double waitStartTime, waitEndTime;

        getTimestamp(NULL, &waitStartTime);
        do {

            // Determine if data available
            struct pollfd pollSock;
            pollSock.fd = navigation.guidance_sock;
            pollSock.events = POLLIN;
            rc = poll(&pollSock, 1, 10);
            if (rc > 0) {
                logDebug(L_INFO, "Navigation: Poll event available\n");
                if (pollSock.revents | POLLIN) {
                    logDebug(L_INFO, "Navigation: Data available on socket\n");
                }
            } else if (rc < 0) {
                logDebug(L_INFO, "Navigation: Poll failed: %s\n",
                        strerror(errno));
            }

            rc = TCPRead(navigation.guidance_sock, &(tcpBuf[numReceived]), 512 - numReceived);
            logDebug(L_INFO, "Navigation: TCPRead returned %d\n", rc);
            usleep(100000);
            if (rc < 0) {
                if (errno == EAGAIN) {
                    logDebug(L_INFO, "Navigation: No TCP data available: %s\n", strerror(errno));
                    usleep(100000);
                } else {
                    logDebug(L_INFO, "Navigation: Failed to read from TCP: %s\n", strerror(errno));
                }
            } else {
                numReceived += rc;

                if (rc > 0) {
                    for (i = 0; i < numReceived; i++) {
                        if (i < 4) {
                            logDebug(L_INFO, "%c", tcpBuf[i]);
                        } else {
                            logDebug(L_INFO, " %02x", tcpBuf[i]);
                        }
                    }
                    logDebug(L_INFO, "\n");
                }

                if (numReceived >= 16) {

                    // See if the message is present in the bytes
                    for (i = 0; i <= numReceived - 16; i++) {

                        if (strncmp((char *) &(tcpBuf[i]), "init", 4) == 0) {
                            // Since it starts with "init", this is the message we want
                            memcpy(&(navigation.startTime), &(tcpBuf[i+4]), 8);
                            memcpy(&(navigation.key), &(tcpBuf[i+12]), 4);

                            messageReceived = 1;
                        }
                    }
                }
            }
            getTimestamp(NULL, &waitEndTime);
            printf("%0.1lf\r", waitEndTime - waitStartTime);
        } while (!messageReceived && numReceived < 512 && ((waitEndTime - waitStartTime) < MAX_TCP_WAIT));

        if (!messageReceived) {
            logDebug(L_INFO, "\nNever received starting conditions from guidance, using defaults\n");

            getTimestamp(NULL, &(navigation.startTime));
            printf("Seeding RNG with %d\n", (unsigned) round(navigation.startTime));
            srand((unsigned) round(navigation.startTime));
            navigation.key = rand();
        } else {
            logDebug(L_INFO, "\nInitial conditions received from guidance in %0.3lf seconds\n",
                    waitEndTime - waitStartTime);
        }
    }

    logDebug(L_INFO, "\n    time = %.6lf; key = %08x\n",
            navigation.startTime, navigation.key);

    time_t startTimeSeconds = (time_t) navigation.startTime;

    // Create a directory to store log files
    char logDirName[512];
    generateFilename(logDirName, 512, &startTimeSeconds,
                "log", "MRFUSION_RUN", navigation.key, "d");

    // Append device logging information to 
    strcat(logDirName, "/VN200");

    // Initialize VN200 device at 50Hz sample frequency, baud rate, for both IMU and GPS packets
    VN200Init(&(navigation.vn200), devname, logDirName, 50, VN200_BAUD,
            VN200_INIT_MODE_BOTH, &startTimeSeconds, navigation.key);

    // Clear any received data from input buffer
    // (could instead verify confirmation message was received)
    VN200FlushInput(&(navigation.vn200));

    // Dispatch reader thread at highest priority
    pthread_attr_t vn200Attr;
    pthread_t vn200Thread;

    rc = ThreadAttrInit(&vn200Attr, 0);
    if (rc < 0) {
        logDebug(L_INFO, "Failed to set thread attributes: %s\n",
                strerror(errno));
        return 1;
    }
    rc = ThreadCreate(&vn200Thread, &vn200Attr, (void *(*)(void *)) &vn200_run, (void *) &navigation);
    if (rc < 0) {
        logDebug(L_INFO, "Failed to start VN200 thread: %s\n",
                strerror(errno));
        return 1;
    }

    usleep(100000);

    if (interactiveMode) {
        logDebug(L_INFO, "Press any key to end\n\n");
    } else {
        logDebug(L_INFO, "Executing until stop command\n\n");
    }

    // Loop Until ending
    int loopContinue = 1;
    int numReceived = 0;
    while (loopContinue) {

        printf("  fx: %.3f  fy: %.3f  fz: %.3f\n",
                imu_packet.accel[0], imu_packet.accel[1], imu_packet.accel[2]);

        printf("  X: %.6f  Y: %.6f  Z: %.6f  fix: %d  sats: %d  \r\033[A",
                gps_packet.PosX, gps_packet.PosY, gps_packet.PosZ,
                gps_packet.GpsFix, gps_packet.NumSats);

        if (interactiveMode) {

            // Get input from user nonblocking
            char input[16];
            rc = read(STDIN_FILENO, input, 16);

            if (rc > 0) {
                loopContinue = 0;
            }

        } else {

            char tcpBuf[512];
            rc = TCPRead(navigation.guidance_sock, (unsigned char *) &(tcpBuf[numReceived]), 512 - numReceived);

            if (rc < 0) {
                logDebug(L_INFO, "Navigation: Failed to read from TCP: %s\n", strerror(errno));
            } else {
                numReceived += rc;

                if (numReceived >= 4) {
                    // See if the message is present in the bytes
                    int i;
                    for (i = 0; i <= numReceived - 4; i++) {

                        if (strncmp((char *) &(tcpBuf[i]), "stop", 4) == 0) {
                            // Stop operation
                            logDebug(L_INFO, "Navigation: Received stop command from guidance\n");
                            loopContinue = 0;
                        }
                    }
                }
            }
        }

    } // while (loopContinue)

    vn200_continueRunning = 0;
    usleep(100000);

    printf("\n\nAttempting to join reader thread...\n");
    int notJoined = 1;
    do {

        rc = ThreadTryJoin(vn200Thread, NULL);
        usleep(100000);

        if (rc == 0 || (rc == -1 && errno != EBUSY)) {
            notJoined = 0;
        }

    } while (notJoined);

    // Clean up device, for completeness
    VN200Destroy(&(navigation.vn200));

    if (interactiveMode) {
        // Restore terminal mode
        setStdinNoncanonical(0);
    } else {
        TCPClose(navigation.guidance_sock);
    }

    return 0;

}

