/***************************************************************************\
 *
 * File:
 * 	VN200.h
 *
 * Description:
 *	Function and type declarations and constants for VN200.c
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/06/2019
 *
 * Revision 0.2
 * 	Last edited 5/30/2019
 * 	Major overhaul unifying GPS and IMU functionality
 *
 ***************************************************************************/

#ifndef __VN200_H
#define __VN200_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "buffer.h"
#include "logger.h"
#include "uart.h"
#include "VN200Struct.h"

#define VN200_DEVNAME "/dev/ttyUSB0"
#define VN200_BAUD 57600
//#define VN200_BAUD 115200

// Device initialization modes
#define VN200_INIT_MODE_GPS 1
#define VN200_INIT_MODE_IMU 2
#define VN200_INIT_MODE_BOTH (VN200_INIT_MODE_GPS|VN200_INIT_MODE_IMU)


int getTimestamp(struct timespec *ts, double *td);

int VN200BaseInit(VN200_DEV *dev, char *devname, int baud);

int VN200Poll(VN200_DEV *dev);

int VN200Consume(VN200_DEV *dev, int num);

int VN200FlushInput(VN200_DEV *dev);

int VN200Command(VN200_DEV *dev, char *buf, int num, int sendChk);

int VN200FlushOutput(VN200_DEV *dev);

int VN200Destroy(VN200_DEV *dev);

int VN200Init(VN200_DEV *dev, char *devname, int baud, int fs, int mode);

#endif

