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

#define STANDARD_DEBUG
#undef VERBOSE_DEBUG

#define DEBUG_OUT_PRINTF
#undef DEBUG_OUT_SYSLOG
#undef DEBUG_OUT_LOGFILE

#endif

