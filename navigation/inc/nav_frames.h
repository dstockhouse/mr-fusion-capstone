/*****************************************************************************\
 *
 * File: 
 * 	nav_frames.h 
 *
 * Description: 
 * 	Functions for converting between nav frames
 *
 * Author:
 *	Joseph Kroeker
 *
 * Revision 0.1
 * 	Last edited 4/21/2020
 *
\*****************************************************************************/

#ifndef __NAV_FRAMES_H
#define __NAV_FRAMES_H

#define LAT_ORIGIN	34.6147979*M_PI/180  // Latitude of tan origin (rad)
#define LONG_ORIGIN    -112.4509615*M_PI/180 // Longitude of tan origin (rad)
#define HEIGHT_ORIGIN   1582.3               // Height of tan origin (m)
#define ECENTRICITY     0.0818               // Ecentricity of earth
#define RADIUS          6378137.0	     // Radius of earth at equator (m)
 
#include <stdio.h>
#include <math.h>

int llh_to_xyz(double* llh, double* r_e__e_c);





