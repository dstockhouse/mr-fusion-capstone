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
 * 	relativeHeading - Desired heading change for controller input (radians)
 * 	leftVelocity    - Pointer to destination for the left motor velocity output (radians/sec)
 * 	rightVelocity   - Pointer to destination for the right motor velocity output (radians/sec)
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int ControllerCalculateActuation(float relativeHeading, float *leftVelocity, float *rightVelocity) {

    if (leftVelocity == NULL || rightVelocity == NULL) {
        return -1;
    }

    return 0;

} // ControllerCalculateActuation(float, float *, float *)

