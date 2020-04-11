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

// Subsystem library headers
#include "control.h"
#include "control_run.h"

int main(int argc, char** argv) {

    int run_rc, rc;

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
    struct sched_param schedParams;
    pid_t pid;
    int schedPriorityMax;

    // Get process PID to configure scheduler
    pid = getpid();

    // Get current scheduler parameters, to be modified
    rc = sched_getparam(pid, &schedParams);
    if (rc == -1) {
        logDebug("sched_getparam() failed: %s\n", strerror(errno));
    }

    // Get maximum priority for realtime scheduler, set main priority to max
    schedPriorityMax = sched_get_priority_max(SCHED_FIFO);
    schedParams.sched_priority = schedPriorityMax;

    // Set to realtime scheduler
    rc = sched_setscheduler(pid, SCHED_FIFO, &schedParams);
    if (rc == -1) {
        logDebug("sched_setscheduler() failed: %s\n", strerror(errno));
        return 1;
    }

    // Dispatch necessary threads and processes
    // Create thread for each serial interface
    // Create thread for main control task

    run_rc = control_run(&control);

    // Join threads

    // Safely shutdown the application

    return run_rc;

} // main()

