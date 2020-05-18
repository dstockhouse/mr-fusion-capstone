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
 * 	Last edited 05/17/2020
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

#include "config.h"
#include "utils.h"
#include "buffer.h"
#include "logger.h"
#include "uart.h"
#include "thread.h"

#include "kangaroo.h"

int odometryContinue = 1;

void *odometry_run(void *param) {

    KANGAROO_PACKET packet;
    int rc;

    // Input parameter is the initialized kangaroo device
    KANGAROO_DEV *dev = (KANGAROO_DEV *) param;

    int lPos, rPos, newRawOdometry = 0, numConsumed;
    double lTimestamp, rTimestamp;

    printf("  Starting odometry thread\n");

    while (odometryContinue) {

        rc = KangarooPoll(dev);
        if (rc < 0) {
            logDebug(L_INFO, "Kangaroo Read thread failed to poll UART Device: %s\n",
                    strerror(errno));
        }

        do {

            numConsumed = 0;

            rc = KangarooParse(dev, &packet);
            if (rc < 0) {
                logDebug(L_INFO, "Kangaroo Read thread failed to parse bytes from packet: %s\n",
                        strerror(errno));
            } else {

                rc = KangarooConsume(dev, rc);
                if (rc < 0) {
                    logDebug(L_INFO, "Failed to consume bytes\n");
                } else {
                    numConsumed = rc;
                }

                if (packet.valid && packet.type == KPT_POS) {
                    if (packet.channel == LEFT_MOTOR_INDEX) {
                        lPos = packet.data;
                        lTimestamp = packet.timestamp;
                        newRawOdometry = 1;
                    } else if (packet.channel == RIGHT_MOTOR_INDEX) {
                        rPos = packet.data;
                        rTimestamp = packet.timestamp;
                        newRawOdometry = 1;
                    }
                } else {
                    // Unexpected packet type
                }

                // If packets have a similar timestamp, then assume they are the same
                if (newRawOdometry) {
                    newRawOdometry = 0;

                    // If less than 50 ms different, assume they are the same reading
                    if (fabs(lTimestamp - rTimestamp) < 0.05) {

                        // Log to file
                        rc = KangarooLogOdometry(dev, lPos, rPos, MIN(lTimestamp, rTimestamp));
                        if (rc < 0) {
                            logDebug(L_INFO, "Unable to log odometry data to file\n");
                        }
                    }
                }

            } // if/else parse succeeded

        } while (numConsumed > 0);

    } // while (odometryContinue)

    printf("  Ending odometry thread\n");

    return (void *) 0;
}

int main(int argc, char **argv) {

    int i, rc;

    // Serial device object
    KANGAROO_DEV dev;

    // Use logDebug(L_DEBUG, ...) just like printf
    // Debug levels are L_INFO, L_DEBUG, L_VDEBUG
    logDebug(L_INFO, "Initializing...\n");

    // Set nonblocking input mode for user control
    setStdinNoncanonical(1);

    // Serial device name may be input, if not use default
    char *devname;
    if (argc > 1) {
        devname = argv[1];
    } else {
        devname = KANGAROO_DEVNAME;
    }

    // Initialize serial device
    rc = KangarooInit(&dev, devname, "log", 9600);
    if (rc != 0) {
        logDebug(L_INFO, "Failed to initialize UART device '%s'\n", devname);
        return rc;
    }

    printf("Arrow keys to change speed and direction.\n");
    printf("Space to stop the robot (slowly)\n");
    printf("'q' to quit the program\n");
    printf("Any other key halts motion immediately\n\n");


    // Dispatch reader thread at highest priority
    pthread_attr_t odometryAttr;
    pthread_t odometryThread;

    rc = ThreadAttrInit(&odometryAttr, 0);
    if (rc < 0) {
        logDebug(L_INFO, "Failed to set thread attributes: %s\n",
                strerror(errno));
        return 1;
    }
    rc = ThreadCreate(&odometryThread, &odometryAttr, &odometry_run, (void *) &dev);
    if (rc < 0) {
        logDebug(L_INFO, "Failed to start odometry thread: %s\n",
                strerror(errno));
        return 1;
    }

    usleep(100000);

    // Loop Until ending
    int loopContinue = 1;
    double speed = 0.0, rotation = 0.0;
    double filtspeed = 0.0, filtrotation = 0.0;
    int escapeMode = 0;
    while (loopContinue) {

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

        printf("  S: %.3f  R: %.3f  M1: %4d  M2: %4d     \r",
                filtspeed, filtrotation, m1speed, m2speed);

        // Drive motors at that speed
        KangarooCommandSpeed(&dev, -m1speed, -m2speed);

        // Request to receive an odometry reading
        KangarooRequestPosition(&dev);

        // Delay for the loop to run more slowly
        struct timespec delaytime;
        delaytime.tv_sec = 0;
        delaytime.tv_nsec = 20000000; // 20 ms
        nanosleep(&delaytime, NULL);

    } // while (loopContinue)

    odometryContinue = 0;
    usleep(100000);

    printf("\n\nAttempting to join reader thread...\n");
    do {

        rc = ThreadTryJoin(odometryThread, NULL);
        usleep(100000);

    } while (rc != 0 && errno == EBUSY);

    // Clean up serial device
    KangarooDestroy(&dev);

    // Restore terminal mode
    setStdinNoncanonical(0);

    return 0;

}

