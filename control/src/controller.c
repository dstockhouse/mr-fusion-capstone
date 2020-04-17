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

#include <stdlib.h>
#include <stdio.h>
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
        printf(" \n Error! No values received for theta_L and/or theta_R. \n");
        return -1;
    }

    float P = KP * delta_heading; // proportional controller segment
    float I = KI * delta_heading; // integral controller segment

    if ((theta_L == 1) && (theta_R == 1)) {
        float omega = P + I; // controller block

        float vL = theta_L - omega * HALF_DRIVE_TRAIN / (2 * RADIUS);
        float vR = theta_R + omega * HALF_DRIVE_TRAIN / (2 * RADIUS);

        float angVelL = vL/RADIUS;
        float andVelR = vR/RADIUS;
    }
    else if (theta_L == 0 || theta_R == 0) {
        float angVelL = 1;
        float andVelR = -1;
    }
    else {
        printf("\n Error! Invalid theta_L or theta_R received. \n");
        return -1;
    }
    return 0;

} // ControllerCalculateActuation(float, float *, float *)

