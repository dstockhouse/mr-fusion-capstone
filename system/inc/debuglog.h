/****************************************************************************
 *
 * File:
 * 	debuglog.h
 *
 * Description:
 * 	Function and type declarations and constants for logger.c
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/20/2019
 *
 * Revision 0.2
 * 	Last edited 2/13/2020
 *
 ***************************************************************************/

#ifndef __DEBUG_LOG_H
#define __DEBUG_LOG_H

#include "config.h"

// Variadic wrapper for appropriate log function
void logDebug(int, const char *fmt, ...);

#endif

