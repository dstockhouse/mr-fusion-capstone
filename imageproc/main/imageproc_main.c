/****************************************************************************
 *
 * File:
 *      imageproc_main.c
 *
 * Description:
 *      Main function for the image processing subsystem
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 04/23/2020
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
#include "debuglog.h"
#include "tcp.h"
#include "thread.h"

// Subsystem library headers
#include "imageproc.h"
#include "imageproc_run.h"

int main(int argc, char** argv) {

    int rc;

    // Object containing parameters for subsystem operation
    // See imageproc.h for definition
    IMAGEPROC_PARAMS imageproc;

    logDebug(L_DEBUG, "ImageProc: Starting imageproc process...\n\n");


    /**** Serial interfaces ****/

    logDebug(L_DEBUG, "ImageProc: Initializing serial interfaces...\n");

    logDebug(L_INFO, "ImageProc:   Skipping serial interface initialization\n");


    /**** Network interfaces ****/

    logDebug(L_DEBUG, "ImageProc: Initializing network interfaces...\n");

    // Initialize socket file descriptors
    imageproc.guidance_sock = -1;
    imageproc.navigation_sock = -1;
    int gSock = TCPClientInit();
    int nSock = TCPClientInit();

    if (gSock == -1 || nSock == -1) {
        logDebug(L_INFO, "ImageProc: Failed to initialize imageproc sockets: %s\n", strerror(errno));
    }

    // Loop until both sockets are connected
    const int MAX_CONNECT_ATTEMPTS = 10000;
    int gConnected = 0, nConnected = 0, numTries = 0; 
    while (!(gConnected && nConnected) && numTries < MAX_CONNECT_ATTEMPTS) {

        // Count attempt number
        numTries++;

        // Attempt to connect to guidance (if not already connected)
        if (!gConnected) {

            logDebug(L_DEBUG, "ImageProc: Attempting to connect to guidance at %s:%d\n", GUIDANCE_IP_ADDR, IMAGEPROC_TCP_PORT);
            rc = TCPClientTryConnect(gSock, GUIDANCE_IP_ADDR, IMAGEPROC_TCP_PORT);
            if (rc != -1) {
                logDebug(L_INFO, "ImageProc: Successful TCP connection to guidance\n");
                gConnected = 1;
                imageproc.guidance_sock = gSock;
            } else if (errno == ECONNREFUSED) {
                logDebug(L_DEBUG, "ImageProc: Unsuccessful connection to guidance, will try again\n");
                // Delay to give other end a chance to start
                usleep(10000);
            } else {
                logDebug(L_INFO, "ImageProc: Could not connect to guidance: %s\n", strerror(errno));
            }
        }

        // Attempt to connect to navigation (if not already connected)
        if (!nConnected) {

            logDebug(L_DEBUG, "ImageProc: Attempting to connect to navigation at %s:%d\n", NAVIGATION_IP_ADDR, IMAGEPROC_TCP_PORT);
            rc = TCPClientTryConnect(nSock, NAVIGATION_IP_ADDR, IMAGEPROC_TCP_PORT);
            if (rc != -1) {
                logDebug(L_INFO, "ImageProc: Successful TCP connection to navigation\n");
                nConnected = 1;
                imageproc.navigation_sock = nSock;
            } else if (errno == ECONNREFUSED) {
                logDebug(L_DEBUG, "ImageProc: Unsuccessful connection to navigation, will try again\n");
                // Delay to give other end a chance to start
                usleep(10000);
            } else {
                logDebug(L_INFO, "ImageProc: Could not connect to navigation: %s\n", strerror(errno));
            }
        }

    } // while (!connected)

    // Too many attempts to establish connection
    if (numTries >= MAX_CONNECT_ATTEMPTS) {
        logDebug(L_INFO, "ImageProc: Exceeded maximum number of TCP connection attempts (%d)\n", MAX_CONNECT_ATTEMPTS);
        logDebug(L_INFO, "ImageProc: TCP connections failed, exiting\n");
        return -1;
    }


    /**** Threads ****/

    logDebug(L_DEBUG, "ImageProc: Starting subsystem threads...\n");

    // For now, only dispatch imageproc thread. Will dispatch hardware interface
    // threads when they are ready
    pthread_attr_t imageprocThreadAttr;
    pthread_t imageprocThread;
    int imageprocReturn;

    // Initialize thread attributes
    rc = ThreadAttrInit(&imageprocThreadAttr, 2);
    if (rc == -1) {
        logDebug(L_INFO, "ImageProc: Failed to initialize imageproc thread attributes: %s\n", strerror(errno));
    }

    // Dispatch threads, give configuration object as argument
    rc = ThreadCreate(&imageprocThread, &imageprocThreadAttr,
            (void *(*)(void *)) &imageproc_run, (void *)&imageproc);
    if (rc == -1) {
        logDebug(L_INFO, "ImageProc: Failed to dispatch imageproc thread: %s\n", strerror(errno));
    }

    logDebug(L_DEBUG, "ImageProc: Threads initialized\n");

    // Join threads
    int i = 0;
    do {
        usleep(100000);
        rc = ThreadTryJoin(imageprocThread, &imageprocReturn);
        i++;
    } while (i < 10 && rc != 0 && errno == EBUSY);

    logDebug(L_DEBUG, "ImageProc: Successfully joined threads.\n");
    logDebug(L_INFO, "\nImageProc: Closing application.\n");

    // Safely shutdown the application
    return 0;

} // main()

