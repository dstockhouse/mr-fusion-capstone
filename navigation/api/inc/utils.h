/***************************************************************************
 *
 * File:
 * 	utils.h
 *
 * Description:
 *	Contains simple utilities used more than once
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 9/28/2019
 *
 ***************************************************************************/

#ifndef __UTILS_H
#define __UTILS_H

// MIN/MAX macro definition
#define MAX(S,T) ((S)>(T)?(S):(T))
#define MIN(S,T) ((S)<(T)?(S):(T))

// Mod that is always positive, unlike %. Use when taking mod of a negative
// number, such as for array bounds
#define MOD(S,T) ((((S)%(T))>0)?((S)%(T)):((S)%(T)+(T)))

#endif

