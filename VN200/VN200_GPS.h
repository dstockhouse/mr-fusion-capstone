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

#include "VN200.h"

#include <inttypes.h>

typedef struct {
	double time;      // 0: Time of the week in seconds
	uint16_t week;    // 1: GPS week
	uint8_t GpsFix;   // 2: GPS fix type. See table below.
	uint8_t NumSats;  // 3: Number of GPS satellites used in solution.
	double Latitude;  // 4: Latitude in degrees.
	double Longitude; // 5: Longitude in degrees.
	double Altitude;  // 6: Altitude above ellipsoid. (WGS84)
	float NedVelX;    // 7: Velocity measurement in north direction.
	float NedVelY;    // 8: Velocity measurement in east direction.
	float NedVelZ;    // 9: Velocity measurement in down direction.
	float NorthAcc;   // 10: North position accuracy estimate. (North)
	float EastAcc;    // 11: East position accuracy estimate. (East)
	float VertAcc;    // 12: Vertical position accuracy estimate. (Down)
	float SpeedAcc;   // 13: Speed accuracy estimate.
	float TimeAcc;    // 14: Time accuracy estimate.  
} GPS_DATA;

int VN200GPSInit(VN200_DEV *dev, int fs);

int VN200GPSParse(VN200_DEV *dev, GPS_DATA *parsedData);

#endif

