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
#include <errno.h>
#include <string.h>
#include <unistd.h>

// Thread management
#include <sched.h>

// Custom library headers
#include "config.h"
#include "debuglog.h"
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


    /**** Serial device loggers ****/

    rc = LogInit(&(control.kangaroo_log), "log", "KANGAROO", LOG_FILEEXT_LOG);
    if (rc < 0) {
        logDebug(L_INFO, "Failed to initialize kangaroo log file (%d)\n", rc);
    }
    rc = LogInit(&(control.encoder_log), "log", "ENCODER", LOG_FILEEXT_LOG);
    if (rc < 0) {
        logDebug(L_INFO, "Failed to initialize encoder log file (%d)\n", rc);
    }


    /**** Serial interfaces ****/

    // Kangaroo Motion Controller (UART)
    //   Init units, communication format
    //   Turn off motors
    //
    // Encoder Buffer (SPI)
    //   Set overflow bounds
    //   Zero pulse count


    /**** Network interfaces ****/

    // Initialize socket file descriptors
    control.guidance_sock = -1;
    control.navigation_sock = -1;
    int gSock = TCPClientInit();
    int nSock = TCPClientInit();

    if (gSock == -1 || nSock == -1) {
        logDebug(L_INFO, "Failed to initialize control sockets: %s\n", strerror(errno));
    }

    // Loop until both sockets are connected
    const int MAX_CONNECT_ATTEMPTS = 10000;
    int gConnected = 0, nConnected = 0, numTries = 0; 
    while (!(gConnected && nConnected) && numTries < MAX_CONNECT_ATTEMPTS) {

        // Count attempt number
        numTries++;

        // Attempt to connect to guidance (if not already connected)
        if (!gConnected) {

            rc = TCPClientTryConnect(gSock, GUIDANCE_IP_ADDR, CONTROL_TCP_PORT);
            if (rc != -1) {
                logDebug(L_INFO, "Successful TCP connection to guidance\n");
                gConnected = 1;
                control.guidance_sock = gSock;
            } else if (errno == ECONNREFUSED) {
                logDebug(L_DEBUG, "Unsuccessful connection to guidance, will try again\n");
                // Possibly delay?
                // usleep(10000);
            } else {
                logDebug(L_INFO, "Could not connect to guidance: %s\n", strerror(errno));
            }
        }

        // Attempt to connect to navigation (if not already connected)
        if (!nConnected) {

            rc = TCPClientTryConnect(nSock, NAVIGATION_IP_ADDR, CONTROL_TCP_PORT);
            if (rc != -1) {
                logDebug(L_INFO, "Successful TCP connection to navigation\n");
                nConnected = 1;
                control.navigation_sock = nSock;
            } else if (errno == ECONNREFUSED) {
                logDebug(L_DEBUG, "Unsuccessful connection to navigation, will try again\n");
            } else {
                logDebug(L_INFO, "Could not connect to navigation: %s\n", strerror(errno));
            }
        }

    } // while (!connected)

    // Too many attempts to establish connection
    if (numTries >= MAX_CONNECT_ATTEMPTS) {
        logDebug(L_INFO, "Exceeded maximum number of TCP connection attempts (%d)\n", MAX_CONNECT_ATTEMPTS);
    }


    /**** Threads ****/
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
        logDebug(L_INFO, "Failed to initialize control thread attributes: %s\n", strerror(errno));
    }

    // Dispatch threads, give configuration object as argument
    // ThreadCreate(&encoderThreadAttr, &encoderThreadAttr, &encoder_run, (void *)&control);
    // ThreadCreate(&kangarooThreadAttr, &kangarooThreadAttr, &kangaroo_run, (void *)&control);
    rc = ThreadCreate(&controlThread, &controlThreadAttr, &control_run, (void *)&control);
    if (rc == -1) {
        logDebug(L_INFO, "Failed to dispatch control thread: %s\n", strerror(errno));
    }

    // Join threads
    // ThreadCreate(&encoderThreadAttr, &encoder_run, (void *)&control);
    // ThreadCreate(&kangarooThreadAttr, &kangaroo_run, (void *)&control);
    rc = ThreadTryJoin(controlThread, &controlReturn);
    if (rc == -1 && errno != EBUSY) {
        logDebug(L_INFO, "Failed to join control thread: %s\n", strerror(errno));
    }

    // Safely shutdown the application
    return 0;

} // main()

