/****************************************************************************
 *
 * File:
 * 	kangaroo.c
 *
 * Description:
 * 	Hardware abstraction for interfacing with the UART Kangaroo Motion Controller
 *
 * Author:
 * 	Duncan Patel
 *
 * Revision 0.1
 * 	Last edited 03/05/2020
 *
 ***************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "config.h"

#include "utils.h"

#include "kangaroo.h"

/**** Function KangarooInit ****
 *
 * Initializes a kangaroo device
 *
 * Arguments: 
 * 	dev - Pointer to Kangaroo object to initialize
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int KangarooInit(KANGAROO_DEV *dev, char *devName, char *logDirName) {

    if (dev == NULL) {
        return -1;
    }

    // Default to save log in current directory
    if (logDirName == NULL) {
        logDirName = ".";
    }

    // Default to common RPi serial device name
    if (devName == NULL) {
        // devName = KANGAROO_DEVNAME;
    }

    // Initialize UART device
    // dev->fd = UARTInit(devName, baud);
    if (dev->fd < 0) {
        logDebug(L_INFO, "Couldn't initialize Kangaroo motion controller UART device\n");
        return -2;
    }

    // Initialize the input and output buffers
    BufferEmpty(&(dev->inbuf));
    BufferEmpty(&(dev->outbuf));

    return 0;

} // KangarooInit(KANGAROO_DEV *, char *, char *)

