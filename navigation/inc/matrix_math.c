/***************************************************************************\
 *
 * File:
 * 	matrix_math.c
 *
 * Description:
 *	Matrix math functions
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

/*
* Function -> invert()
* Purpose -> invert a given 3x3 Matrix
* Inputs -> In = Input 3x3 matrix
* Outputs -> Out = Output 3x3 matrix
*/
int invert(double** Out, double** In) {
    // Iterate through all indices of 3x3 matrix
    for(int i = 0; i<3; i++) {
        for(int j = 0; j<3; j++) {
            Out[i][j] = In[j][i]; // Swap rows and columns
	   }
    }
    return 1;
} // invert(double** Out, double** In)

/*
* Function -> mtimes_3_1()
* Purpose -> Multiply a 3x3 matrix and a 3x1 Matrix
* Inputs -> Mat_3_3 = 3x3 Matrix 
*			Mat_3_1 = 3x1 Matrix
* Output -> Out = Resultant 3x1 Matrix
*			rv = 1 if successful
*/
int mtimes_3_1(double** Mat_3_3, double* Mat_3_1, double* Out) {
    // Iterate through all indices in the 3x3 matrix
    for(int i = 0; i<3; i++) {
	   for(int j = 0; j<3; j++) {
	   	    // Multiply each column by the corresponding index of the 3x1 
	   		// and sum the rows
            Out[i] += Mat_3_3[i][j] * Mat_3_1[j];   
	   } 
    }
    return 1;
} // mtimes(double** Mat_3_3, double* Mat_3_1, double* Out)