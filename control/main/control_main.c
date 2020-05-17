/****************************************************************************
 *
 * File:
 *      control_main.c
 *
 * Description:
 *      Main function for the control subsystem
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 04/10/2020
 *
 ***************************************************************************/

// Standard headers
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

// Thread management
#include <sched.h>

// Custom library headers
#include "config.h"
#include "utils.h"
#include "tcp.h"
#include "thread.h"

// Subsystem library headers
#include "control.h"
#include "control_run.h"
#include "kangaroo_run.h"
#include "encoder_run.h"

int main(int argc, char** argv) {

    int rc;

    // Object containing parameters for subsystem operation
    // See control.h for definition
    CONTROL_PARAMS control;

    logDebug(L_DEBUG, "Control: Starting control process...\n\n");

    /**** Serial device loggers ****/

    logDebug(L_DEBUG, "Control: Initializing serial interfaces...\n");

    rc = LogInit(&(control.kangaroo_log), "log", "KANGAROO", LOG_FILEEXT_LOG);
    if (rc < 0) {
        logDebug(L_INFO, "Control: Failed to initialize kangaroo log file (%d)\n", rc);
    }
    rc = LogInit(&(control.encoder_log), "log", "ENCODER", LOG_FILEEXT_LOG);
    if (rc < 0) {
        logDebug(L_INFO, "Control: Failed to initialize encoder log file (%d)\n", rc);
    }


    /**** Serial interfaces ****/

    logDebug(L_INFO, "Control:   Skipping serial interface initialization\n");

    // Kangaroo Motion Controller (UART)
    //   Init units, communication format
    //   Turn off motors
    //
    // Encoder Buffer (SPI)
    //   Set overflow bounds
    //   Zero pulse count


    /**** Network interfaces ****/

    logDebug(L_DEBUG, "Control: Initializing network interfaces...\n");

    // Initialize socket file descriptors
    control.guidance_sock = -1;
    control.navigation_sock = -1;
    int gSock = TCPClientInit();
    int nSock = TCPClientInit();

    if (gSock == -1 || nSock == -1) {
        logDebug(L_INFO, "Control: Failed to initialize control sockets: %s\n", strerror(errno));
    }

    // Loop until both sockets are connected
    const int MAX_CONNECT_ATTEMPTS = 10000;
    int gConnected = 0, nConnected = 0, numTries = 0; 
    while (!(gConnected && nConnected) && numTries < MAX_CONNECT_ATTEMPTS) {

        // Count attempt number
        numTries++;

        // Attempt to connect to guidance (if not already connected)
        if (!gConnected) {

            logDebug(L_DEBUG, "Control: Attempting to connect to guidance at %s:%d\n", GUIDANCE_IP_ADDR, CONTROL_TCP_PORT);
            rc = TCPClientTryConnect(gSock, GUIDANCE_IP_ADDR, CONTROL_TCP_PORT);
            if (rc != -1) {
                logDebug(L_INFO, "Control: Successful TCP connection to guidance\n");
                gConnected = 1;
                control.guidance_sock = gSock;
            } else if (errno == ECONNREFUSED) {
                logDebug(L_DEBUG, "Control: Unsuccessful connection to guidance, will try again\n");
                // Delay to give other end a chance to start
                usleep(10000);
            } else {
                logDebug(L_INFO, "Control: Could not connect to guidance: %s\n", strerror(errno));
            }
        }

        // Attempt to connect to navigation (if not already connected)
        if (!nConnected) {

            logDebug(L_DEBUG, "Control: Attempting to connect to navigation at %s:%d\n", NAVIGATION_IP_ADDR, CONTROL_TCP_PORT);
            rc = TCPClientTryConnect(nSock, NAVIGATION_IP_ADDR, CONTROL_TCP_PORT);
            if (rc != -1) {
                logDebug(L_INFO, "Control: Successful TCP connection to navigation\n");
                nConnected = 1;
                control.navigation_sock = nSock;
            } else if (errno == ECONNREFUSED) {
                logDebug(L_DEBUG, "Control: Unsuccessful connection to navigation, will try again\n");
                // Delay to give other end a chance to start
                usleep(10000);
            } else {
                logDebug(L_INFO, "Control: Could not connect to navigation: %s\n", strerror(errno));
            }
        }

    } // while (!connected)

    // Too many attempts to establish connection
    if (numTries >= MAX_CONNECT_ATTEMPTS) {
        logDebug(L_INFO, "Control: Exceeded maximum number of TCP connection attempts (%d)\n", MAX_CONNECT_ATTEMPTS);
        logDebug(L_INFO, "Control: TCP connections failed, exiting\n");
        return -1;
    }


    /**** Threads ****/

    logDebug(L_DEBUG, "Control: Starting subsystem threads...\n");

    // For now, only dispatch control thread. Will dispatch hardware interface
    // threads when they are ready
    pthread_attr_t controlThreadAttr /*, kangarooThreadAttr, encoderThreadAttr*/;
    pthread_t controlThread /*, kangarooThread, encoderThread*/;
    int controlReturn /*, kangarooReturn, encoderReturn*/;

    // Initialize thread attributes
    // ThreadAttrInit(&encoderThreadAttr, 0);
    // ThreadAttrInit(&kangarooThreadAttr, 1);
    rc = ThreadAttrInit(&controlThreadAttr, 2);
    if (rc == -1) {
        logDebug(L_INFO, "Control: Failed to initialize control thread attributes: %s\n", strerror(errno));
    }

    // Dispatch threads, give configuration object as argument
    // ThreadCreate(&encoderThreadAttr, &encoderThreadAttr, &encoder_run, (void *)&control);
    // ThreadCreate(&kangarooThreadAttr, &kangarooThreadAttr, &kangaroo_run, (void *)&control);
    rc = ThreadCreate(&controlThread, &controlThreadAttr,
            (void *(*)(void *)) &control_run, (void *)&control);
    if (rc == -1) {
        logDebug(L_INFO, "Control: Failed to dispatch control thread: %s\n", strerror(errno));
    }

    logDebug(L_DEBUG, "Control: Threads initialized\n");

    // Join threads
    int i = 0;
    do {
        usleep(100000);
        rc = ThreadTryJoin(controlThread, &controlReturn);
        i++;
    } while (i < 10 && rc != 0 && errno == EBUSY);

    logDebug(L_DEBUG, "Control: Successfully joined threads.\n");
    logDebug(L_INFO, "\nControl: Closing application.\n");

    // Safely shutdown the application
    return 0;

} // main()

