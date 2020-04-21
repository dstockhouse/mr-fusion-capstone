
/***************************************************************************\
 *
 * File:
 * 	nav_frame.c
 *
 * Description:
 *	Frame transformations for navigation sensors
 * 
 * Author:
 * 	Joseph Kroeker  
 *
 * Revision 0.1
 * 	Last edited 4/21/2020
 *
 ***************************************************************************/

#include <stdio.h>
#include <math.h>

int llh_to_xyz(double* llh, double* r_e__e_c) {
    // Radiuis of earth at coordinates
    double R_e = RADIUS / sqrt(1 - exp(2) * pow(sin(llh[1]),2));
    
    r_e__e_c[1] = (R_e + llh[3]) * cos(llh[1]) * cos(llh[2]);	// X Coord (m)
    r_e__e_c[2] = (R_e + llh[3]) * cos(llh[1]) * sin(llh[2]);   // Y Coord (m)
    r_e__e_c[3] = ((1 - exp(2))*R_e + llh[3]) * sin(llh[1]);	// Z Coord (m) 

    return 1;
}
