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

#include "kangaroo.h"

typedef struct {

    // TCP Socket file descriptors
    int guidance_sock, navigation_sock;

    KANGAROO_DEV kangaroo;

    // Time zero for all logging
    double startTime;

    // Key to identify log files
    unsigned key;

} CONTROL_PARAMS;

#endif // __CONTROL_H

