/***************************************************************************\
 *
 * File:
 * 	VN200_IMU.h
 *
 * Description:
 *	Function and type declarations and constants for VN200_IMU.c
 * 
 * Author:
 * 	Joseph Kroeker
 *
 * Revision 0.1
 * 	Last edited 4/01/2019
 *
 * Revision 0.2
 * 	Last edited 5/07/2019
 *
 ***************************************************************************/

#ifndef __VN200_IMU_H
#define __VN200_IMU_H

#include "logger.h"
#include "VN200Struct.h"
#include "VN200.h"

int VN200IMUInit(VN200_DEV *dev, char *devname, int fs);

int VN200IMUPacketParse(unsigned char *buf, int len, IMU_DATA *data);

int VN200IMULogParsed(LOG_FILE *log, IMU_DATA *data);

#endif

