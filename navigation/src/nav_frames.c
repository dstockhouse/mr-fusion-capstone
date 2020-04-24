
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

#include "nav_frames.h"

int llh_to_xyz(double* llh, double* r_e__e_c) {
    // Radiuis of earth at coordinates
    double R_e = RADIUS / sqrt(1 - exp(2) * pow(sin(llh[0]),2));
    
    r_e__e_c[0] = (R_e + llh[2]) * cos(llh[0]) * cos(llh[1]);	// X Coord (m)
    r_e__e_c[1] = (R_e + llh[2]) * cos(llh[0]) * sin(llh[1]);   // Y Coord (m)
    r_e__e_c[2] = ((1 - exp(1))*R_e + llh[2]) * sin(llh[0]);	// Z Coord (m) 

    return 1;
}

int ECEF_llh_to_tan(double* llh, double* r_t__t_b, double** C_e__t) {
    // Convert from ECEF llh to 
    double r_e__e_t[3];
    double r_e__e_b[3];
    double r_e__t_b[3];
    double C_e__n[3][3];
    double C_n__e[3][3]; 
    int rv = 0;

    // Determine the tangential xyz location 
    rv = llh_to_xyz(llh, r_e__e_t);
    rv = llh_to_xyz(llh, r_e__e_b);
    for(int i = 0; i<3; i++) {
        r_e__t_b[i] = r_e__e_b[i] - r_e__e_t[i];
    }

    // Determine the tangential rotation matrix
    rv = llh_to_C_e__n(llh, C_e__n);
    rv = invert(C_n__e, C_e__n);
    rv = mtimes(C_n__e, r_e__t_b, r_t__t_b);
    C_e__t = C_e__n;
    return 1;
}

int ECEF_xyz_to_tan(double* xyz, double* r_e__t_b, double** C_e__t) {

    return 1;
}

int llh_to_C_e__n(double* llh, double** C_e__n) {
    C_e__n[0][0] = -cos(llh[1])*sin(llh[0]);
    C_e__n[0][1] = -sin(llh[1]);
    C_e__n[0][2] = -cos(llh[1])*cos(llh[0]);
    C_e__n[1][0] = -sin(llh[1])*sin(llh[0]);
    C_e__n[1][1] =  cos(llh[1]);
    C_e__n[1][2] = -sin(llh[1])*cos(llh[0]);
    C_e__n[2][0] =  cos(llh[0]);
    C_e__n[2][1] =  0;
    C_e__n[2][2] = -sin(llh[0]);
    return 1;
}

int invert(double** Out, double** In) {
    for(int i = 0; i<3; i++) {
        for(int j = 0; j<3; j++) {
            Out[i][j] = In[j][i];
	}
    }
    return 1;
}

int mtimes(double** Mat_3_3, double* Mat_3_1, double* Out) {
    for(int i = 0; i<3; i++) {
	for(int j = 0; j<3; j++) {
            Out[i] += Mat_3_3[i][j] * Mat_3_1[j];   
	} 
    }
    return 1;
}
