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

#include "VN200.h"

typedef struct {
	double compass[3]; // compass (x,y,z) Gauss
	double accel[3]; // accel (x,y,z) m/s^2
	double gyro[3]; // gyro (x, y, z) rad/s
	double temp; // temp C
	double baro; // pressure kPa
} IMU_DATA;

int VN200IMUInit(VN200_IMU *dev);

int VN200IMUParse(VN200_IMU *dev, IMU_DATA *data);

#endif
