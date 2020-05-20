/****************************************************************************
 *
 * File:
 *      vn200_run.c
 *
 * Description:
 *      Function header for vn200_run thread
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 04/23/2020
 *
 ***************************************************************************/

#ifndef VN200_RUN_H
#define VN200_RUN_H

#include "navigation.h"

// Stores received packet data temporarily, immediately after parsing
// Also used to share data between threads (unsafe but temporary behavior)
extern GPS_DATA gps_packet;
extern IMU_DATA imu_packet;

// Allows main thread to stop running thread
extern int vn200_continueRunning;

int vn200_run(NAVIGATION_PARAMS *navigation);

#endif // VN200_RUN_H


