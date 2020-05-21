/***************************************************************************\
 *
 * File:
 * 	kangaroo_main.c
 *
 * Description:
 *	Tests the Kangaroo device functionality
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 05/19/2020
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

#include "kangaroo.h"

#include "control.h"
#include "control_run.h"
#include "kangaroo_run.h"
#include "encoder_run.h"

int main(int argc, char **argv) {

    int interactiveMode = 0;
    int i, rc;

    // Instance of subsystem control structure
    CONTROL_PARAMS control;

    // Use logDebug(L_DEBUG, ...) just like printf
    // Debug levels are L_INFO, L_DEBUG, L_VDEBUG
    logDebug(L_INFO, "Control: Initializing...\n");

    // Serial device name may be input, if not use default
    char *devname;
    if (argc > 1) {
        devname = argv[1];
    } else {
        devname = KANGAROO_DEVNAME;
    }

    // Pause to allow spawning process a chance to start server
    sleep(1);

    // Initialize sockets
    // This deployment only initialize in non-interactive mode and only guidance
    control.guidance_sock = -1;
    control.navigation_sock = -1;

    if (!interactiveMode) {
        // See skeleton_control_main for proper and complete tcp initialization
        logDebug(L_DEBUG, "Control: Attempting to connect to guidance at %s:%d\n", GUIDANCE_IP_ADDR, CONTROL_TCP_PORT);
        int gSock = TCPClientInit();
        if (gSock == -1) {
            logDebug(L_INFO, "Control: Failed to initialize control sockets: %s\n", strerror(errno));
        }

        // Loop until all sockets are connected
        const int MAX_CONNECT_ATTEMPTS = 100;
        int gConnected = 0, numTries = 0; 
        while (!gConnected && numTries < MAX_CONNECT_ATTEMPTS) {

            // Count attempt number
            numTries++;

            // Attempt to connect to guidance (if not already connected)
            if (!gConnected) {

                rc = TCPClientTryConnect(gSock, GUIDANCE_IP_ADDR, CONTROL_TCP_PORT);
                if (rc != -1) {
                    logDebug(L_INFO, "Control: Successful TCP connection to guidance\n");
                    gConnected = 1;
                    control.guidance_sock = gSock;
                } else if (errno == ECONNREFUSED) {
                    logDebug(L_DEBUG, "Control: Unsuccessful connection to guidance, will try again\n");
                    // Delay to give other end a chance to start
                    usleep(100000);
                } else {
                    logDebug(L_INFO, "Control: Could not connect to guidance: %s\n", strerror(errno));
                }
            }

        } // while (!connected && tries remaining)

        // Too many attempts to establish connection
        if (numTries >= MAX_CONNECT_ATTEMPTS) {

            logDebug(L_INFO, "Control: Exceeded maximum number of TCP connection attempts (%d)\n", MAX_CONNECT_ATTEMPTS);

            TCPClose(gSock);

            // Since we couldn't connect, assume user has terminal
            interactiveMode = 1;
        }
    }


    // Get configuration ID's for this run (start time and numeric key)
    if (interactiveMode) {

        // Set nonblocking input mode for user control
        setStdinNoncanonical(1);

        // Zero time is now
        getTimestamp(NULL, &(control.startTime));

        // Get random key for logging
        srand((unsigned) round(control.startTime));
        printf("Seeding RNG with %d\n", (unsigned) round(control.startTime));
        control.key = rand();

    } else {

        // Wait for initialization message from guidance
        logDebug(L_INFO, "Control: Waiting for initialization message from guidance...\n");

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
            pollSock.fd = control.guidance_sock;
            pollSock.events = POLLIN;
            rc = poll(&pollSock, 1, 10);
            if (rc > 0) {
                if (pollSock.revents | POLLIN) {
                    // logDebug(L_INFO, "Control: Data available on socket\n");
                }
            } else if (rc < 0) {
                logDebug(L_INFO, "Control: Poll failed: %s\n",
                        strerror(errno));
            }

            rc = TCPRead(control.guidance_sock, &(tcpBuf[numReceived]), 512 - numReceived);
            usleep(100000);
            if (rc < 0) {
                if (errno == EAGAIN) {
                    logDebug(L_INFO, "Control: No TCP data available: %s\n", strerror(errno));
                    usleep(100000);
                } else {
                    logDebug(L_INFO, "Control: Failed to read from TCP: %s\n", strerror(errno));
                }
            } else {
                numReceived += rc;

/*
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
*/

                if (numReceived >= 16) {

                    // See if the message is present in the bytes
                    for (i = 0; i <= numReceived - 16; i++) {

                        if (strncmp((char *) &(tcpBuf[i]), "init", 4) == 0) {
                            // Since it starts with "init", this is the message we want
                            memcpy(&(control.startTime), &(tcpBuf[i+4]), 8);
                            memcpy(&(control.key), &(tcpBuf[i+12]), 4);

                            messageReceived = 1;
                        }
                    }
                }
            }
            getTimestamp(NULL, &waitEndTime);
            // printf("%0.1lf\r", waitEndTime - waitStartTime);
        } while (!messageReceived && numReceived < 512 && ((waitEndTime - waitStartTime) < MAX_TCP_WAIT));

        if (!messageReceived) {
            logDebug(L_INFO, "\nControl: Never received starting conditions from guidance, using defaults\n");

            getTimestamp(NULL, &(control.startTime));
            printf("Seeding RNG with %d\n", (unsigned) round(control.startTime));
            srand((unsigned) round(control.startTime));
            control.key = rand();
        } else {
            logDebug(L_INFO, "\nControl: Initial conditions received from guidance in %0.3lf seconds\n",
                    waitEndTime - waitStartTime);
        }
    }

    logDebug(L_INFO, "\n    time = %.6lf; key = %08x\n",
            control.startTime, control.key);

    time_t startTimeSeconds = (time_t) control.startTime;

    // Create a directory to store log files
    char logDirName[512];
    generateFilename(logDirName, 512, &startTimeSeconds,
                "log", "MRFUSION_RUN", control.key, "d");

    // Append device logging information to 
    strcat(logDirName, "/control");


    // Initialize serial device
    rc = KangarooInit(&(control.kangaroo), devname, logDirName, KANGAROO_BAUD,
            &startTimeSeconds, control.key);
    if (rc != 0) {
        logDebug(L_INFO, "Control: Failed to initialize UART device '%s'\n", devname);
        return rc;
    }


    // Dispatch reader thread at highest priority
    pthread_attr_t encoderAttr;
    pthread_t encoderThread;

    rc = ThreadAttrInit(&encoderAttr, 0);
    if (rc < 0) {
        logDebug(L_INFO, "Control: Failed to set thread attributes: %s\n",
                strerror(errno));
        return 1;
    }
    rc = ThreadCreate(&encoderThread, &encoderAttr, (void *(*)(void *)) &encoder_run, (void *) &control);
    if (rc < 0) {
        logDebug(L_INFO, "Control: Failed to start encoder thread: %s\n",
                strerror(errno));
        return 1;
    }

    // Delay before starting operation
    sleep(2);

    if (interactiveMode) {
        printf("Arrow keys to change speed and direction.\n");
        printf("Space to stop the robot (slowly)\n");
        printf("'q' to quit the program\n");
        printf("Any other key halts motion immediately\n\n");
    } else {
        logDebug(L_INFO, "Control: Executing until stop command\n\n");
    }

    // Buffer for TCP inputs from guidance
    BYTE_BUFFER tcpBuf;
    BufferEmpty(&tcpBuf);

    // Loop Until ending
    int loopContinue = 1, loopCount = 0;;
    double speed = 0.0, rotation = 0.0;
    double filtspeed = 0.0, filtrotation = 0.0;
    int escapeMode = 0;
    while (loopContinue) {

        // Keep track of how many loops we execute
        loopCount++;

        if (interactiveMode) {
            // Get input from user nonblocking
            char input[16];
            rc = read(STDIN_FILENO, input, 16);

            // speed = 0.0;
            // rotation = 0.0;

            for (i = 0; i < rc; i++) {
                switch (input[i]) {

                    // Escape code
                    case '\033':
                        escapeMode = 1;
                        break;

                        // Home exits
                    case 'F':
                        if (escapeMode) {
                            loopContinue = 0;
                            escapeMode = 0;
                        }
                        break;

                        // Linear speed control
                    case 'A': // Up arrow
                        if (escapeMode) {
                            speed += 0.2;
                            escapeMode = 0;
                        }
                        break;
                    case 'B': // Down arrow
                        if (escapeMode) {
                            speed -= 0.2;
                            escapeMode = 0;
                        }
                        break;

                        // Angular speed control
                    case 'C': // Left arrow
                        if (escapeMode) {
                            rotation += 0.2;
                            escapeMode = 0;
                        }
                        break;
                    case 'D': // Right arrow
                        if (escapeMode) {
                            rotation -= 0.2;
                            escapeMode = 0;
                        }
                        break;

                    case ' ':
                        speed = 0;
                        rotation = 0;
                        break;

                    case 'q':
                        // Exit loop
                        loopContinue = 0;
                        break;

                        // Ignore parts of special characters
                    case '[':
                    case '~':
                        // No input
                        break;

                    default:
                        filtspeed = 0;
                        filtrotation = 0;
                        speed = 0;
                        rotation = 0;
                        escapeMode = 0;
                        break;
                }
            }

        } else {

            char tcpTempBuf[512];
            rc = TCPRead(control.guidance_sock, (unsigned char *) tcpTempBuf, 512);

            if (rc < 0) {
                logDebug(L_INFO, "Control: Failed to read from TCP: %s\n", strerror(errno));
            } else if (rc > 0) {

                // Add characters to buffer
                BufferAddArray(&tcpBuf, (unsigned char *) tcpTempBuf, rc);

                int numParsed = 0;
                if (BufferLength(&tcpBuf) >= 4) {

                    int i;
                    logDebug(L_VDEBUG, "Control: Buffer contents: ");
                    for (i = 0; i < BufferLength(&tcpBuf); i++) {
                        logDebug(L_VDEBUG, "%c ", BufferIndex(&tcpBuf, i));
                    }
                    logDebug(L_VDEBUG, "\n");

                    // Parse any known message from received bytes
                    int increment;
                    for (i = 0; i <= BufferLength(&tcpBuf) - 4; i += increment) {
                        increment = 1;

                        if (BufferIndex(&tcpBuf, i  ) == 's' &&
                            BufferIndex(&tcpBuf, i+1) == 't' &&
                            BufferIndex(&tcpBuf, i+2) == 'o' &&
                            BufferIndex(&tcpBuf, i+3) == 'p') {

                            // Stop operation
                            logDebug(L_INFO, "Control: Received stop command from guidance\n");
                            loopContinue = 0;

                            numParsed = i + 4;
                            increment = 4;

                        } else if (BufferIndex(&tcpBuf, i  ) == 'c' &&
                                   BufferIndex(&tcpBuf, i+1) == 't' &&
                                   BufferIndex(&tcpBuf, i+2) == 'l' &&
                                   BufferIndex(&tcpBuf, i+3) == 'x') {

                            // Halt motion
                            logDebug(L_INFO, "Control: Received halt command from guidance\n");
                            filtspeed = 0;
                            filtrotation = 0;
                            speed = 0;
                            rotation = 0;

                            numParsed = i + 4;
                            increment = 4;

                        } else if (BufferIndex(&tcpBuf, i  ) == 'c' &&
                                   BufferIndex(&tcpBuf, i+1) == 't' &&
                                   BufferIndex(&tcpBuf, i+2) == 'l' &&
                                   BufferIndex(&tcpBuf, i+3) == 's') {

                            // Speed command
                            logDebug(L_VDEBUG, "Control: Received speed command from guidance\n");
                            if (BufferLength(&tcpBuf) >= i + 12) {
                                unsigned char tempBuf[8];
                                BufferCopy(&tcpBuf, tempBuf, i + 4, 8);
                                memcpy(&speed, tempBuf, 8);

                                numParsed = i + 12;
                                increment = 12;
                            }

                        } else if (BufferIndex(&tcpBuf, i  ) == 'c' &&
                                   BufferIndex(&tcpBuf, i+1) == 't' &&
                                   BufferIndex(&tcpBuf, i+2) == 'l' &&
                                   BufferIndex(&tcpBuf, i+3) == 'r') {

                            // Rotation command
                            logDebug(L_VDEBUG, "Control: Received rotation command from guidance\n");
                            if (BufferLength(&tcpBuf) >= i + 12) {
                                unsigned char tempBuf[8];
                                BufferCopy(&tcpBuf, tempBuf, i + 4, 8);
                                memcpy(&rotation, tempBuf, 8);

                                numParsed = i + 12;
                                increment = 12;
                            }
                        }
                    }

                    // Remove those bytes from the buffer
                    logDebug(L_VDEBUG, "Removing %d/%d: ", numParsed, BufferLength(&tcpBuf));
                    for (i = 0; i < BufferLength(&tcpBuf); i++) {
                        logDebug(L_VDEBUG, "%c", BufferIndex(&tcpBuf, i));
                    }
                    logDebug(L_VDEBUG, "\n");
                    BufferRemove(&tcpBuf, numParsed);

                }

            } // if TCP data received

        } // if/else (interactiveMode)

        if (speed > 0.8) {
            speed = 0.8;
        } else if (speed < -0.8) {
            speed = -0.8;
        }
        if (rotation > 0.8) {
            rotation = 0.8;
        } else if (rotation < -0.8) {
            rotation = -0.8;
        }

        // Apply a low pass filter
        int filtersize = 16;
        filtspeed = filtspeed * (filtersize-1) / filtersize + speed / filtersize;
        filtrotation = filtrotation * (filtersize-1) / filtersize + rotation / filtersize;

        // Calculate speed to drive each motor
        int m1speed = (int)(filtspeed * 1000 - filtrotation * 500);
        int m2speed = (int)(filtspeed * 1000 + filtrotation * 500);

        // Drive motors at that speed
        KangarooCommandSpeed(&(control.kangaroo), -m1speed, -m2speed);

        if (loopCount % 5 == 0) {
            // Request to receive an odometry reading
            KangarooRequestPosition(&(control.kangaroo));
        }

        // Print message for the parent process
        logDebug(L_INFO, "  S: %.3f  R: %.3f  M1: %4d  M2: %4d     \r",
                filtspeed, filtrotation, m1speed, m2speed);

        // Delay for the loop to run more slowly
        struct timespec delaytime;
        delaytime.tv_sec = 0;
        delaytime.tv_nsec = 20000000; // 20 ms
        nanosleep(&delaytime, NULL);

    } // while (loopContinue)

    encoder_continueRunning = 0;
    usleep(100000);

    printf("\n\nAttempting to join encoder reader thread...\n");
    int notJoined = 1;
    do {

        rc = ThreadTryJoin(encoderThread, NULL);
        usleep(100000);

        if (rc == 0 || (rc == -1 && errno != EBUSY)) {
            notJoined = 0;
        }

    } while (notJoined);

    // Clean up serial device
    KangarooDestroy(&(control.kangaroo));

    if (interactiveMode) {
        // Restore terminal mode
        setStdinNoncanonical(0);
    } else {
        TCPClose(control.guidance_sock);
    }

    return 0;

}

