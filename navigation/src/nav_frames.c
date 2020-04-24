
/***************************************************************************\
 *
 * File:
 * 	nav_frames.c
 *
 * Description:
 *	Frame transformations for navigation sensors
 * 
 * Author:
 * 	Joseph Kroeker  
 *
 * Revision 0.1
 * 	Last edited 4/23/2020
 *
 ***************************************************************************/
#include <stdio.h>
#include <math.h>

#include "matrix_math.h"
#include "nav_frames.h"

/*
* Function -> llh_to_xyz()
* Purpose -> Transform from Lat/Long/Height to ECEF XYZ 
* Inputs -> llh = Lat/Long/Height 3x1 Vector
* Outputs -> r_e__e_c = xyz 3x1 vector from earth center to given coordinates
*            rv = 1 if successful
*/
int llh_to_xyz(double* llh, double* r_e__e_c) {
    // Radiuis of earth at coordinates
    double R_e = RADIUS / sqrt(1 - exp(2) * pow(sin(llh[0]),2));
    
    r_e__e_c[0] = (R_e + llh[2]) * cos(llh[0]) * cos(llh[1]);	// X Coord (m)
    r_e__e_c[1] = (R_e + llh[2]) * cos(llh[0]) * sin(llh[1]);   // Y Coord (m)
    r_e__e_c[2] = ((1 - exp(1))*R_e + llh[2]) * sin(llh[0]);	// Z Coord (m) 

    return 1;
} // llh_to_xyz(double* llh, double* r_e__e_c)

/*
* Function -> xyz_to_llh()
* Purpose -> Transform from ECEF XYZ to Lat/Long/Height
* Inputs -> xyz = xyz coordinates in ECEF frame 3x1 Vector
* Outputs -> llh = Lat/Long/Height 3x1 Vector
*            rv = 1 if successful
*/
int xyz_to_llh(double* xyz, double* llh) {
    // NOTE: Avoid using this function if possible, it is computationally 
    // more complex than starting in llh and going to xyz and yeilds less 
    // accurate results. If possible output llh from sensors, if not 
    // possible then consider cranking up the iteration count
    double rr = 0; // Length on Equatorial plane
    double R_e = RADIUS; // Radius to point on Earth from center
    double sin_Lat; 

    xyz[2] = 0; // Initialize the height to the radius of the earth

    llh[1] = atan2(xyz[1], xyz[0]); // Determine set value for Longitude
    rr = sqrt(pow(xyz[0],2) + pow(xyz[1],2));
    // Iterate through approximating Lattitude and Height to get most accurate value
    for(int i = 0; i<10; i++) {
        sin_Lat = xyz[2]/((1-exp(2))*R_e + xyz[2]);
        xyz[0] = atan((xyz[2]+exp(2)*R_e*sin_Lat)/rr); // Aproximate Latitude
        R_e = RADIUS/sqrt(1-exp(2)*pow(sin_Lat,2));
	xyz[2] = rr/cos(xyz[0] - R_e); // Aproximate Height
    }
} // xyz_to_llh(double* xyz, double* llh)

/*
* Function -> ECEF_llh_to_tan()
* Purpose -> Transform from ECEF Lat/Long/Height to XYZ in tan frame
* Inputs -> llh = Lat/Long/Height 3x1 Vector
* Outputs -> r_t__t_b = XYZ coordinates of body in tan frame
*            rv = 1 if successful
*/
int ECEF_llh_to_tan(double* llh, double* r_t__t_b, double** C_e__t) {
    // Origin Lat/Long/Height
    double llh_origin[3] = {LAT_ORIGIN, LONG_ORIGIN, HEIGHT_ORIGIN};
    double r_e__e_t[3]; // Vector from ECEF frame to Tan frame
    double r_e__e_b[3]; // Vector from ECEF frame to body frame
    double r_e__t_b[3]; // Vector from tan frame to body frame (relative to ECEF)
    double C_e__n[3][3]; // Rotation Matrix from earth to body frame
    double C_n__e[3][3];  // Rotation Matrix from body to earth
    int rv = 0; // Return value from different functions

    // Determine the tangential xyz location 
    rv = llh_to_xyz(llh_origin, r_e__e_t); // determine xyz of origin
    rv = llh_to_xyz(llh, r_e__e_b); // determine 
    // Subtract matrices to determine vector between tan frame and body
    for(int i = 0; i<3; i++) {
        r_e__t_b[i] = r_e__e_b[i] - r_e__e_t[i];
    }

    // Determine the tangential rotation matrix
    rv = llh_to_C_e__n(llh, C_e__n); // Rotation matrix to body frame
    rv = invert(C_n__e, C_e__n); // Invert to transform from earth to body
    rv = mtimes_3_1(C_n__e, r_e__t_b, r_t__t_b); // Determine XYZ in tan frame
    C_e__t = C_e__n;
    return 1;
} // ECEF_llh_to_tan(double* llh, double* r_t__t_b, double** C_e__t)

/*
* Function -> ECEF_xyz_to_tan()
* Purpose -> Transform from ECEF XYZ to Tan frame XYZ
* Inputs -> r_e__e_b = XYZ coordinates of body from ECEF frame
* Outputs -> r_t__t_b = XYZ coordinates of body in tan frame 
*            C_e__t = Rotation from ECEF to tan frame
*            rv = 1 if successful
*/
int ECEF_xyz_to_tan(double* r_e__e_b, double* r_t__t_b, double** C_e__t) {
    // Lat/Long/Height of the Origin 
    double llh_origin[3] = {LAT_ORIGIN, LONG_ORIGIN, HEIGHT_ORIGIN}; 
    double llh[3]; // Lat/Long/Height of location
    double r_e__e_t[3]; // Vector from ECEF frame to tan frame
    double r_e__t_b[3]; // Vector from tan frame to body frame (Relative to ECEF)
    double C_e__n[3][3]; // Rotation Matrix from ECEF to body frame
    double C_n__e[3][3]; // Rotation Matrix from body to ECEF frame
    int rv = 0;

    // Determine the tangential xyz location 
    rv = llh_to_xyz(llh_origin, r_e__e_t);
    rv = xyz_to_llh(r_e__e_b, llh); // Not preferable (Appoximation)
    // Determine vector between body and tan frame
    for(int i = 0; i<3; i++) {
        r_e__t_b[i] = r_e__e_b[i] - r_e__e_t[i];
    }

    // Determine the tangential rotation matrix
    rv = llh_to_C_e__n(llh, C_e__n); // Determine Rotation matrix
    rv = invert(C_n__e, C_e__n); // Invert matrix
    rv = mtimes_3_1(C_n__e, r_e__t_b, r_t__t_b); // Find final vector
    C_e__t = C_e__n;
    return 1;
} // ECEF_xyz_to_tan(double* r_e__e_b, double* r_t__t_b, double** C_e__t)

/*
* Function -> llh_to_C_e__n()
* Purpose -> find the ECEF to body frame rotation matrices from Lat/Long/Height
* Inputs -> llh = Lat/Long/Height 3x1 Matrix
* Outputs -> C_e__n = Rotation matrix from ECEF to body frame
*/
int llh_to_C_e__n(double* llh, double** C_e__n) {
    // 3x3 Matrix defined for C_e_n
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
} // llh_to_C_e__n(double* llh, double** C_e__n)


