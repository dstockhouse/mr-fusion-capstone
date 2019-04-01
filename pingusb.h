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
 * 	Last edited 4/01/2019
 *
\***************************************************************************/

#ifndef __PINGUSB_H
#define __PINGUSB_H

#include "uart.h"
#include "buffer.h"
#include "logger.h"
#include "ADS_B.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#define PINGUSB_RECV_DEV "/dev/ttyUSB0"
// #define PINGUSB_RECV_DEV "/dev/ttyACM0"
#define PINGUSB_RECV_BAUD 57600

typedef struct {
	int fd;
	BYTE_BUFFER inbuf;
	MsgData246 packetData;
	MsgHeader packetHeader;
	LOG_FILE logFile;
} PINGUSB_RECV;

int pingUSBInit(PINGUSB_RECV *dev);

int pingUSBPoll(PINGUSB_RECV *dev);

int pingUSBParse(PINGUSB_RECV *dev);

int pingUSBConsume(PINGUSB_RECV *dev, int num);

int pingUSBDestroy(PINGUSB_RECV *dev);

#endif

