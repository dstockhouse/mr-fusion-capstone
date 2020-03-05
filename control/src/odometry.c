/****************************************************************************
 *
 * File:
 * 	odometry.c
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

#include "odometry.h"

/**** Function OdometryWheelVelocityFromPulses ****
 *
 * Determines how fast one motor moved based on number of pulses over time
 *
 * Arguments: 
 * 	pulseCount - Number of encoder pulses (relative)
 * 	timestep   - Amount of time pulse count was collected
 *
 * Return value:
 * 	Wheel angular velocity in radians/second
 */
float OdometryWheelVelocityFromPulses(int pulseCount, float timestep) {

    float wheelVelocity = 0.0; // Replace with function(s) of pulseCount and timestep

    return wheelVelocity;

} // OdometryWheelVelocityFromPulses(int, float)

