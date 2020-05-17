/***************************************************************************
 *
 * File:
 * 	utils.h
 *
 * Description:
 *	Contains simple utilities used for the system
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 09/28/2019
 *
 * Revision 0.2
 * 	Last edited 05/17/2020
 * 	Added getTimestamp previously in VN200 code and integrated logDebug
 *
 ***************************************************************************/

#ifndef __UTILS_H
#define __UTILS_H

#include <time.h> // For struct timespec

// MIN/MAX macro definition
#define MAX(S,T) ((S)>(T)?(S):(T))
#define MIN(S,T) ((S)<(T)?(S):(T))

// Mod that is always positive, unlike %. Use when taking mod of a negative
// number, such as for array bounds
#define MOD(S,T) ((((S)%(T))<0)?((S)%(T)+(T)):((S)%(T)))

// Wrapper for system call gettime
int getTimestamp(struct timespec *ts, double *td);

// Variadic wrapper for appropriate log function
void logDebug(int debuglevel, const char *fmt, ...);

// Sets std input to noncanonical mode
void setStdinNoncanonical(int set);

#endif // __UTILS_H

