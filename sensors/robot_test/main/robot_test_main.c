/***************************************************************************\
 *
 * File:
 * 	kangaroo_main.c
 *
 * Description:
 *	Tests the Kangaroo functionality
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 05/03/2020
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

// Use ncurses library
// #include <ncurses.h>

#include "config.h"
#include "utils.h"
#include "buffer.h"
#include "logger.h"
#include "uart.h"

#include "kangaroo.h"

// #define KANGAROO_DEVNAME "/dev/ttyAMA0"

int setSpeed(int fd, int motor, int speed) {

    char command[512];

    int len = snprintf(command, 512, "%d,s%d\r\n", motor, speed);

    int rc = UARTWrite(fd, command, len);

    return rc;
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
    rc = KangarooInit(&dev, devname, NULL, 9600);
    if (rc != 0) {
        logDebug(L_INFO, "Failed to initialize UART device '%s'\n", devname);
        return rc;
    }

    printf("Arrow keys to change speed and direction.\n");
    printf("Space to stop the robot (slowly)\n");
    printf("'q' to quit the program\n");
    printf("Any other key halts motion immediately\n\n");

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
                case 'A':
                    if (escapeMode) {
                        speed += 0.2;
                        escapeMode = 0;
                    }
                    break;
                case 'B':
                    if (escapeMode) {
                        speed -= 0.2;
                        escapeMode = 0;
                    }
                    break;

                    // Angular speed control
                case 'C':
                    if (escapeMode) {
                        rotation += 0.2;
                        escapeMode = 0;
                    }
                    break;
                case 'D':
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

        printf("  S: %.3f  R: %.3f   M1: %4d   M2: %4d      \r",
                filtspeed, filtrotation, m1speed, m2speed);

        // Drive motors at that speed
        KangarooCommandSpeed(&dev, -m1speed, -m2speed);

        struct timespec delaytime;
        delaytime.tv_sec = 0;
        delaytime.tv_nsec = 20000000; // 20 ms
        nanosleep(&delaytime, NULL);

    } // while (loopContinue)

    // Clean up serial device
    KangarooDestroy(&dev);

    // Restore terminal mode
    setStdinNoncanonical(0);

    return 0;

}

