/***************************************************************************\
 *
 * File:
 * 	kangaroo.h
 *
 * Description:
 *	Function and type declarations and constants for kangaroo.c
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/03/2020
 *
 ***************************************************************************/

#ifndef __KANGAROO_H
#define __KANGAROO_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "buffer.h"
#include "logger.h"
#include "uart.h"
#include "vn200_struct.h"

#define KANGAROO_DEVNAME "/dev/ttyUSB0"
#define KANGAROO_BAUD 57600
//#define KANGAROO_BAUD 115200


int getTimestamp(struct timespec *ts, double *td);

int KangarooInit(KANGAROO_DEV *dev, char *devname, int baud);

int KangarooPoll(KANGAROO_DEV *dev);

int KangarooConsume(KANGAROO_DEV *dev, int num);

int KangarooFlushInput(KANGAROO_DEV *dev);

int KangarooCommand(KANGAROO_DEV *dev, char *buf, int num, int sendChk);

int KangarooFlushOutput(KANGAROO_DEV *dev);

int KangarooDestroy(KANGAROO_DEV *dev);

#endif // __KANGAROO_H

