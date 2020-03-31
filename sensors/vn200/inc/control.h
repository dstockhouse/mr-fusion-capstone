/***************************************************************************
 *
 * File:
 * 	control.h
 *
 * Description:
 *	Constants to control how the software is compiled.
 *	Include all constants expected by the source code, and either
 *	#define to enable a feature or #undef to disable.
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 9/28/2019
 *
 ***************************************************************************/

#ifndef __CONTROL_H
#define __CONTROL_H

#define L_VVDEBUG   -1
#define L_VDEBUG    0
#define L_DEBUG     1
#define L_INFO      2

#define CONTROL_DEBUG_L_MASK    L_DEBUG

#define DEBUG_OUT_PRINTF
#undef DEBUG_OUT_SYSLOG
#define DEBUG_OUT_LOGFILE

#endif

