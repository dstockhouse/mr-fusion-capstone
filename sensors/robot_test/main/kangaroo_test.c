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
#include <ncurses.h>

#include "utils.h"
#include "buffer.h"
#include "logger.h"
#include "debuglog.h"
#include "uart.h"

#define KANGAROO_DEVNAME "/dev/ttyAMA0"

int cursesInit();
int cursesClose();

int setSpeed(int fd, int motor, int speed) {

    char command[512];

    int len = snprintf(command, 512, "%d,s%d\r\n", motor, speed);

    int rc = UARTWrite(fd, command, len);

    return rc;
}

int main(int argc, char **argv) {

    int i, rc;

    // File descriptor for serial device
    int fd;

    // Use logDebug(L_DEBUG, ...) just like printf
    // Debug levels are L_INFO, L_DEBUG, L_VDEBUG
    logDebug(L_INFO, "Initializing...\n");

    rc = cursesInit();
    if (rc) {
        return rc;
    }

    char *devname;
    if (argc > 1) {
        devname = argv[1];
    } else {
        devname = KANGAROO_DEVNAME;
    }

    fd = UARTInit(devname, 9600);
    if (fd < 0) {
        logDebug(L_INFO, "Failed to initialize UART device '%s'\n", devname);
        return fd;
    }

    // Send commands to initialize device
    char *command;

    command = "1,start\r\n";
    UARTWrite(fd, command, strlen(command));
    command = "2,start\r\n";
    UARTWrite(fd, command, strlen(command));

    // command = "1,units360deg=420lines\r\n";
    // UARTWrite(fd, command, strlen(command));
    // command = "2,units360deg=420lines\r\n";
    // UARTWrite(fd, command, strlen(command));
    command = "1,units798mm=420lines\r\n";
    UARTWrite(fd, command, strlen(command));
    command = "2,units798mm=420lines\r\n";
    UARTWrite(fd, command, strlen(command));

    // Loop Until ending
    int loopContinue = 1;
    double speed = 0.0, rotation = 0.0;
    while (loopContinue) {

        // Get single character input from user
        int input = getch();
        flushinp();

        speed = 0.0;
        rotation = 0.0;

        switch (input) {

            // Linear speed control
            case KEY_UP:
                speed = 0.5;
                break;
            case KEY_DOWN:
                speed = -0.5;
                break;

            // Angular speed control
            case KEY_LEFT:
                rotation = 0.25;
                break;
            case KEY_RIGHT:
                rotation = -0.25;
                break;

            case 'q':
                // Exit loop
                loopContinue = 0;
                break;

            default:
                break;
        }

        // Calculate speed to drive each motor
        int m1speed = (int)(speed * 1000 - rotation * 500);
        int m2speed = (int)(speed * 1000 + rotation * 500);

        printw("  S: %.3f  R: %.3f   M1: %d   M2: %d                          \r",
                speed, rotation, m1speed, m2speed);

        // Drive motors at that speed
        setSpeed(fd, 1, m1speed);
        setSpeed(fd, 2, m2speed);

    } // while (loopContinue)

    // Turn off motors
    command = "1,powerdown\r\n";
    UARTWrite(fd, command, strlen(command));
    command = "2,powerdown\r\n";
    UARTWrite(fd, command, strlen(command));

    // Clean up device, for completeness
    UARTClose(fd);

    cursesClose();

    return 0;

}

int cursesInit() {

    initscr();      // Initializes stdscr
    if(stdscr == NULL)
    {
        endwin();
        logDebug(L_INFO, "Failed to init ncurses\n");
        return 2;
    }

    cbreak();       // Disable char buffering
    noecho();       // Don't echo input to screen
    notimeout(stdscr, TRUE);    // No delay after pressing escape
    keypad(stdscr, TRUE);   // Allow special characters

    return 0;

}

int cursesClose() {
    endwin();

    return 0;
}
