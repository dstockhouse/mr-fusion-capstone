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

#include "buffer.h"
#include "logger.h"

#define KANGAROO_DEVNAME "/dev/ttyAMA0"
#define KANGAROO_BAUD 9600

#define LEFT_MOTOR_INDEX    1
#define RIGHT_MOTOR_INDEX   2

// Represents a kangaroo device connected through UART
typedef struct {

    int fd;

	BYTE_BUFFER inbuf;
	BYTE_BUFFER outbuf;

    LOG_FILE logFile;
    LOG_FILE logFileParsed;

} KANGAROO_DEV;

// Types of packets that the kangaroo might send
typedef enum {
    KPT_ERROR,
    KPT_POS,
    KPT_SPEED
} KANGAROO_PACKET_TYPE;

typedef struct {

    int channel;
    KANGAROO_PACKET_TYPE type;
    int data;
    int valid;
    double timestamp;

} KANGAROO_PACKET;

int KangarooInit(KANGAROO_DEV *dev, char *devName, char *logDirName, int baud);

int KangarooCommandSpeed(KANGAROO_DEV *dev, int lSpeed, int rSpeed);

int KangarooRequestPosition(KANGAROO_DEV *dev);

int KangarooPoll(KANGAROO_DEV *dev);

int KangarooParse(KANGAROO_DEV *dev, KANGAROO_PACKET *packet);

int KangarooLogOdometry(KANGAROO_DEV *dev, int lPos, int rPos, double timestamp);

int KangarooConsume(KANGAROO_DEV *dev, int num);

int KangarooFlushInput(KANGAROO_DEV *dev);

int KangarooCommand(KANGAROO_DEV *dev, char *buf, int num, int sendChk);

int KangarooDestroy(KANGAROO_DEV *dev);

#endif // __KANGAROO_H

