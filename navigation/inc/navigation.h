/****************************************************************************
 *
 * File:
 *      navigation.h
 *
 * Description:
 *      Constants and types for the navigation subsystem
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 04/23/2020
 *
 ***************************************************************************/

#ifndef __NAVIGATION_H
#define __NAVIGATION_H

#include "vn200_struct.h"

typedef struct {

    // TCP Socket file descriptors
    int guidance_sock, control_sock, imageproc_sock;

    // Serial device file descriptors
    VN200_DEV vn200;

} NAVIGATION_PARAMS;

#endif // __NAVIGATION_H

