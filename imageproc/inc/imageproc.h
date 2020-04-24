/****************************************************************************
 *
 * File:
 *      imageproc.h
 *
 * Description:
 *      Constants and types for the imageproc subsystem
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 04/09/2020
 *
 ***************************************************************************/

#ifndef __IMAGEPROC_H
#define __IMAGEPROC_H

typedef struct {

    // TCP Socket file descriptors
    int guidance_sock, navigation_sock;

} IMAGEPROC_PARAMS;

#endif // __IMAGEPROC_H


