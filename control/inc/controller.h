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

int ControllerCalculateActuation(float relativeHeading, float *leftVelocity, float *rightVelocity);

#endif // __CONTROLLER_H

