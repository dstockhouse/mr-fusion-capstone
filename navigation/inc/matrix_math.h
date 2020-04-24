/*****************************************************************************\
 *
 * File: 
 * 	matrix_math.h 
 *
 * Description: 
 * 	Matrix math functions
 *
 * Author:
 *	Joseph Kroeker
 *
 * Revision 0.1
 * 	Last edited 4/23/2020
 *
\*****************************************************************************/
#ifndef __MATRIX_MATH_H
#define __MATRIX_MATH_H

#include <stdio.h>
#include <math.h>

int invert(double** Out, double** In);
int mtimes_3_1(double** Mat_3_3, double* Mat_3_1, double* Out);

#endif // __MATRIX_MATH_H
