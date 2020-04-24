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
int xyz_to_llh(double* xyz, double* llh);
int ECEF_llh_to_tan(double* llh, double* r_t__t_b, double** C_e__t);
int ECEF_xyz_to_tan(double* r_e__e_b, double* r_t__t_b, double** C_e__t);
int llh_to_C_e__n(double* llh, double** C_e__n);
int invert(double** Out, double** In);
int mtimes(double** Mat_3_3, double* Mat_3_1, double* Out);

#endif



