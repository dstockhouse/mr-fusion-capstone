/***************************************************************************\
 *
 * File:
 * 	VN200_GPS.h
 *
 * Description:
 *	Function and type declarations and constants for VN200_GPS.c
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 5/06/2019
 *
 ***************************************************************************/

#ifndef __VN200_GPS_H
#define __VN200_GPS_H

#include "logger.h"
#include "VN200Struct.h"
#include "VN200.h"

int VN200GPSInit(VN200_DEV *dev, char *devname, int fs);

int VN200GPSPacketParse(unsigned char *buf, int len, GPS_DATA *data);

int VN200GPSLogParsed(LOG_FILE *log, GPS_DATA *data);

#endif

