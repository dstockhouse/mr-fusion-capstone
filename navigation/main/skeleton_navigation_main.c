/****************************************************************************
 *
 * File:
 *      navigation_main.c
 *
 * Description:
 *      Main function for the navigation subsystem
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

// Socket constants
#include <netinet/in.h>

// Thread management
#include <sched.h>

// Custom library headers
#include "config.h"
#include "utils.h"
#include "tcp.h"
#include "thread.h"

// Subsystem library headers
#include "navigation.h"
#include "navigation_run.h"
#include "vn200_run.h"

int main(int argc, char** argv) {

    int rc;

    // Object containing parameters for subsystem operation
    // See navigation.h for definition
    NAVIGATION_PARAMS navigation;

    logDebug(L_DEBUG, "Navigation: Starting navigation process...\n\n");


    /**** Serial interfaces ****/

    logDebug(L_DEBUG, "Navigation: Initializing serial interfaces...\n");

    logDebug(L_INFO, "Navigation:   Skipping serial interface initialization\n");

    // VN200Init(&(navigation.vn200), VN200_DEV, 50, 115200, VN200_INIT_BOTH);

    // VN200 GPS & IMU (UART)
    //   Init data modes
    //   Set to output data continuously


    /**** Network interfaces ****/

    logDebug(L_DEBUG, "Navigation: Initializing network interfaces...\n");

    // Initialize socket file descriptors
    navigation.guidance_sock = -1;
    navigation.control_sock = -1;
    navigation.imageproc_sock = -1;
    int gSock = TCPClientInit();
    int cSock = TCPServerInit(NAVIGATION_IP_ADDR, CONTROL_TCP_PORT);
    TCPSetNonBlocking(cSock);
    int ipSock = TCPServerInit(NAVIGATION_IP_ADDR, IMAGEPROC_TCP_PORT);
    TCPSetNonBlocking(ipSock);

    if (gSock == -1 || cSock == -1 || ipSock == -1) {
        logDebug(L_INFO, "Navigation: Failed to initialize navigation sockets: %s\n", strerror(errno));
    }

    // Loop until all sockets are connected
    const int MAX_CONNECT_ATTEMPTS = 10000;
    int gConnected = 0, cConnected = 0, ipConnected = 0, numTries = 0; 
    while (!(gConnected && cConnected && ipConnected) && numTries < MAX_CONNECT_ATTEMPTS) {

        // Count attempt number
        numTries++;

        // Attempt to connect to guidance (if not already connected)
        if (!gConnected) {

            logDebug(L_DEBUG, "Navigation: Attempting to connect to guidance at %s:%d\n", GUIDANCE_IP_ADDR, NAVIGATION_TCP_PORT);
            rc = TCPClientTryConnect(gSock, GUIDANCE_IP_ADDR, NAVIGATION_TCP_PORT);
            if (rc != -1) {
                logDebug(L_INFO, "Navigation: Successful TCP connection to guidance\n");
                gConnected = 1;
                navigation.guidance_sock = gSock;
            } else if (errno == ECONNREFUSED) {
                logDebug(L_DEBUG, "Navigation: Unsuccessful connection to guidance, will try again\n");
                // Delay to give other end a chance to start
                usleep(10000);
            } else {
                logDebug(L_INFO, "Navigation: Could not connect to guidance: %s\n", strerror(errno));
            }
        }

        // Attempt to connect to control (if not already connected)
        if (!cConnected) {

            logDebug(L_DEBUG, "Navigation: Attempting to accept connection from control at port %d\n", CONTROL_TCP_PORT);
            rc = TCPServerTryAccept(cSock);
            if (rc != -1) {
                logDebug(L_INFO, "Navigation: Successful TCP connection to control\n");
                cConnected = 1;
                navigation.control_sock = rc;
            } else if (errno == EAGAIN) {
                logDebug(L_DEBUG, "Navigation: Unsuccessful connection to control, will try again\n");
                // Delay to give other end a chance to start
                usleep(10000);
            } else {
                logDebug(L_INFO, "Navigation: Could not connect to control: %s\n", strerror(errno));
            }
        }

        // Attempt to connect to image processing (if not already connected)
        if (!ipConnected) {

            logDebug(L_DEBUG, "Navigation: Attempting to accept connection from image processing at port %d\n", IMAGEPROC_TCP_PORT);
            rc = TCPServerTryAccept(ipSock);
            if (rc != -1) {
                logDebug(L_INFO, "Navigation: Successful TCP connection to image processing\n");
                ipConnected = 1;
                navigation.imageproc_sock = rc;
            } else if (errno == EAGAIN) {
                logDebug(L_DEBUG, "Navigation: Unsuccessful connection to image processing, will try again\n");
                // Delay to give other end a chance to start
                usleep(10000);
            } else {
                logDebug(L_INFO, "Navigation: Could not connect to image processing: %s\n", strerror(errno));
            }
        }

    } // while (!connected && tries remaining)

    // Too many attempts to establish connection
    if (numTries >= MAX_CONNECT_ATTEMPTS) {
        logDebug(L_INFO, "Navigation: Exceeded maximum number of TCP connection attempts (%d)\n", MAX_CONNECT_ATTEMPTS);
        logDebug(L_INFO, "Navigation: TCP connections failed, exiting\n");
        return -1;
    }


    /**** Threads ****/

    logDebug(L_DEBUG, "Navigation: Starting subsystem threads...\n");

    // For now, only dispatch navigation thread. Will dispatch hardware interface
    // threads when they are ready
    pthread_attr_t navigationThreadAttr /*, vn200ThreadAttr*/;
    pthread_t navigationThread /*, vn200Thread*/;
    int navigationReturn /*, vn200Return*/;

    // Initialize thread attributes
    // ThreadAttrInit(&vn200ThreadAttr, 0);
    rc = ThreadAttrInit(&navigationThreadAttr, 1);
    if (rc == -1) {
        logDebug(L_INFO, "Navigation: Failed to initialize navigation thread attributes: %s\n", strerror(errno));
    }

    // Dispatch threads, give configuration object as argument
    // ThreadCreate(&vn200ThreadAttr, &vn200ThreadAttr, &vn200_run, (void *)&navigation);
    rc = ThreadCreate(&navigationThread, &navigationThreadAttr,
            (void *(*)(void *)) &navigation_run, (void *)&navigation);
    if (rc == -1) {
        logDebug(L_INFO, "Navigation: Failed to dispatch navigation thread: %s\n", strerror(errno));
    }

    logDebug(L_DEBUG, "Navigation: Threads initialized\n");

    // Join threads
    int i = 0;
    do {
        usleep(100000);
        rc = ThreadTryJoin(navigationThread, &navigationReturn);
        i++;
    } while (i < 10 && rc != 0 && errno == EBUSY);

    logDebug(L_DEBUG, "Navigation: Successfully joined threads.\n");
    logDebug(L_INFO, "\nNavigation: Closing application.\n");

    // Safely shutdown the application
    return 0;

} // main()

