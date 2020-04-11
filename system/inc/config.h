/***************************************************************************
 *
 * File:
 * 	config.h
 *
 * Description:
 *	Constants to configure how the software is compiled.
 *	Include all constants expected by the source code, and either
 *	#define to enable a feature or #undef to disable.
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 03/18/2020
 *
 ***************************************************************************/

#ifndef MR_FUSION_SYSTEM_CONFIG
#define MR_FUSION_SYSTEM_CONFIG

// Debug logging parameters
#define L_VVDEBUG   -1
#define L_VDEBUG    0
#define L_DEBUG     1
#define L_INFO      2

#define CONTROL_DEBUG_L_MASK    L_DEBUG

#define DEBUG_OUT_PRINTF
#undef DEBUG_OUT_SYSLOG
#undef DEBUG_OUT_LOGFILE


// Placeholders. Fill in with real data during test and integration
#define GUIDANCE_IP_ADDR    "192.168.1.1"
#define NAVIGATION_IP_ADDR  "192.168.1.3"
#define CONTROL_IP_ADDR     "192.168.1.2"
#define IMAGEPROC_IP_ADDR   "192.168.1.4"

// Ports that each client subsystem uses to seek connections to other subsystems
// Ex. Control connects to guidance navigation at the same port
#define GUIDANCE_TCP_PORT       31400
#define NAVIGATION_TCP_PORT     31402
#define CONTROL_TCP_PORT        31401
#define IMAGEPROC_TCP_PORT      31403

#endif // MR_FUSION_SYSTEM_CONFIG

