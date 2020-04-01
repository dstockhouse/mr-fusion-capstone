/****************************************************************************
 *
 * File:
 * 	controller.c
 *
 * Description:
 * 	Logic for determining how to drive the motors given an input
 *
 * Author:
 * 	Connor Rockwell
 *
 * Revision 0.1
 * 	Last edited 03/05/2020
 *
 ***************************************************************************/

#include <math.h>

#include "debuglog.h"

#include "controller.h"

/**** Function ControllerCalculateActuation ****
 *
 * Determines how fast to drive each of the motors based on the input heading
 *
 * Arguments: 
 * 	delta_heading - Desired heading change for controller input (radians)
 * 	speed         - Desired linear velocity for controller input
 * 	theta_L    - Pointer to destination for the left motor velocity output (radians/sec)
 * 	theta_R   - Pointer to destination for the right motor velocity output (radians/sec)
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int ControllerCalculateActuation(float delta_heading, float speed, float *theta_L, float *theta_R) {

    // If no values are received, return as failure
    if (theta_L == NULL || theta_R == NULL) {
        return -1;
    }

    float omega = delta_heading * CONTROLLER_KP; // rate of angular change of robot

    float vL = speed - (HALF_DRIVE_TRAIN * omega);  // speed command for left motor (meters/sec)
    float vR = speed + (HALF_DRIVE_TRAIN * omega);  // speed command for right motor (meters/sec)

    *theta_L = vL / RADIUS; // speed command for left motor (radians/second)
    *theta_R = vR / RADIUS; // speed command for right motor (radians/second)

    return 0;

} // ControllerCalculateActuation(float, float *, float *)

