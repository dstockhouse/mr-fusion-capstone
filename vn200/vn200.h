/***************************************************************************\
 *
 * File:
 * 	vn200.h
 *
 * Description:
 *	Function and type declarations and constants for vn200.c
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 4/20/2019
 *
 ***************************************************************************/

#ifndef __VN200_H
#define __VN200_H

#include "../uart/uart.h"
#include "../buffer/buffer.h"
#include "../logger/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#define VN200_DEVNAME "/dev/ttyUSB0"
#define VN200_BAUD 57600

typedef struct {
	int fd;
	BYTE_BUFFER inbuf;
	BYTE_BUFFER outbuf;
	LOG_FILE logFile;
} VN200_DEV;

int vn200BaseInit(VN200_DEV *dev);

int vn200Poll(VN200_DEV *dev);

int vn200Consume(VN200_DEV *dev, int num);

int vn200Command(VN200_DEV *dev, char *buf, int num);

int vn200Flush(VN200_DEV *dev);

int vn200Destroy(VN200_DEV *dev);

#endif

