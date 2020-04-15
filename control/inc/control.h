/****************************************************************************
 *
 * File:
 *      control.h
 *
 * Description:
 *      Constants and types for the control subsystem
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 04/09/2020
 *
 ***************************************************************************/

#ifndef __CONTROL_H
#define __CONTROL_H

#include "logger.h"

typedef struct {

    // TCP Socket file descriptors
    int guidance_sock, navigation_sock;

    // Serial device file descriptors
    int kangaroo_fd, encoder_fd;

    // Serial device loggers
    LOG_FILE kangaroo_log, encoder_log;

} CONTROL_PARAMS;

#endif // __CONTROL_H

