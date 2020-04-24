/*****************************************************************************\
 *
 * File: temp_test.c
 *
 * Purpose: Temporary Test file as I don't have cgreen installed properly
 *
 * Author: Joseph Kroeker
 *
 * Revision: 0.1 Last Modified: 4/24/2020
 *
\*****************************************************************************/
#include <stdio.h>
#include <math.h>

#include "../inc/matrix_math.h"
#include "../inc/nav_frames.h"

/*
 * Function -> main()
 * Purpose -> Test nav_frame functions to ensure completion
 * Inputs -> Void
 * Outputs -> 1 if successful
 */
int main(void) {
    printf("Initializing\n");
    double llh[3] = {LAT_ORIGIN, LONG_ORIGIN, HEIGHT_ORIGIN};
    printf("llh set\n");
    double C_e__n[3][3];
    printf("C_e__n set\n");
    double r_e__e_c[3][3];
    printf("r_e__e_c set\n");
    double r_t__t_b[3];

    printf("Starting\n");  
    ECEF_llh_to_tan(llh, r_t__t_b, C_e__n);
    printf("ECEF llh done\n");
    llh_to_xyz(llh, r_e__e_c);
    printf("llhs to xyz done\n");
    ECEF_xyz_to_tan(r_e__e_c, r_t__t_b, C_e__n);
    printf("done\n");

}

