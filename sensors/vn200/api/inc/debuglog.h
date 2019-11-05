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
 ***************************************************************************/

#ifndef __DEBUG_LOG_H
#define __DEBUG_LOG_H

#include "control.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// Variadic wrapper for appropriate log function
int logDebug(const char *fmt, ...);

#endif

