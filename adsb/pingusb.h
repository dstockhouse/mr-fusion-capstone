/****************************************************************************\
 *
 * File:
 * 	pingusb.h
 *
 * Description:
 * 	Function and type declarations and constants for pingusb.c
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Split UART and ADB functionality
 * 	Last edited 4/01/2019
 *
 * Revision 0.2
 * 	Renamed structure type to PINGUSB_DEV
 * 	Last edited 4/20/2019
 *
\***************************************************************************/

#ifndef __PINGUSB_H
#define __PINGUSB_H

#include "../uart/uart.h"
#include "../buffer/buffer.h"
#include "../logger/logger.h"
#include "adsb_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#define PINGUSB_DEVNAME "/dev/ttyUSB0"
// #define PINGUSB_DEVNAME "/dev/ttyACM0"
#define PINGUSB_BAUD 57600

typedef struct {
	int fd;
	BYTE_BUFFER inbuf;
	MsgData246 packetData;
	MsgHeader packetHeader;
	LOG_FILE logFile;
	LOG_FILE logFileParsed;
} PINGUSB_DEV;

int pingUSBInit(PINGUSB_DEV *dev);

int pingUSBPoll(PINGUSB_DEV *dev);

int pingUSBParse(PINGUSB_DEV *dev);

int pingUSBConsume(PINGUSB_DEV *dev, int num);

int pingUSBDestroy(PINGUSB_DEV *dev);

#endif

