/****************************************************************************
 *
 * File:
 * 	controller.h
 *
 * Description:
 * 	Function and type declarations and constants for controller.c
 *
 * Author:
 * 	Connor Rockwell
 *
 * Revision 0.1
 * 	Last edited 03/05/2020
 *
 ***************************************************************************/

#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#define CONTROLLER_KP 0.5
#define RADIUS 0.5524
#define HALF_DRIVE_TRAIN 0.1524

int ControllerCalculateActuation(float relativeHeading, float *leftVelocity, float *rightVelocity);

#endif // __CONTROLLER_H

