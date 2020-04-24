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
 * 	Last edited 04/20/2020
 *
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
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
int ControllerCalculateActuation(double delta_heading, bool speed, double *delta_heading_previous_sum, double *theta_L, double *theta_R) {

    // If no values are received, return as failure
    if (theta_L == NULL || theta_R == NULL) {
        printf(" \n Error! No values received for theta_L and/or theta_R. \n");
        return -1;
    }

    double integral = *delta_heading_previous_sum;

    double P = KP * delta_heading;                                 // proportional controller segment
    double I = KI * (delta_heading + integral); // integral controller segment

    if (speed == 1) {
        double omega = P + I; // controller block

        double vL = 1.0 - omega * HALF_DRIVE_TRAIN / (2 * RADIUS);
        double vR = 1.0 + omega * HALF_DRIVE_TRAIN / (2 * RADIUS);

        *theta_L = vL/RADIUS;
        *theta_R = vR/RADIUS;
        return 0;
    }
    else if (speed == 0) {
        *theta_L = 1;
        *theta_R = -1;
        return 0;
    }
    else {
        printf(" \n Error! Invalid input received. \n ");
        return -1;
    }

    *delta_heading_previous_sum+=delta_heading;

} // ControllerCalculateActuation(float, float *, float *)

