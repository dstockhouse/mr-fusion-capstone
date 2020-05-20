/****************************************************************************
 *
 * File:
 * 	kangaroo.h
 *
 * Description:
 * 	Function and type declarations and constants for kangaroo.c
 *
 * Author:
 * 	Duncan Patel
 *
 * Revision 0.1
 * 	Last edited 03/05/2020
 *
 ***************************************************************************/

#ifndef __KANGAROO_H
#define __KANGAROO_H

#include "buffer.h"
#include "logger.h"

typedef struct {

    int fd; // UART file descriptor

    BYTE_BUFFER inbuf;  // Input data buffer
    BYTE_BUFFER outbuf;     // Output data buffer

    LOG_FILE logFile; // Raw data log file

    int baud; // Baud rate
    int fs; // Sampling Frequency

} KANGAROO_DEV;

int KangarooInit(KANGAROO_DEV *dev, char *devName, char *logDirName);

#endif // __KANGAROO_H

