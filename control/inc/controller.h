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

#define KP 4.0
#define KI 0.2

#define RADIUS 0.5524
#define HALF_DRIVE_TRAIN 0.1524

int ControllerCalculateActuation(double delta_heading, bool speed, double *delta_heading_previous_sum, double *theta_L, double *theta_R);

#endif // __CONTROLLER_H

