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
#define GUIDANCE_IP_ADDR    "XXX.XXX.XXX.XXX"
#define NAVIGATION_IP_ADDR  "XXX.XXX.XXX.XXX"
#define CONTROL_IP_ADDR     "XXX.XXX.XXX.XXX"
#define DRAGONBOARD_IP_ADDR "XXX.XXX.XXX.XXX"

// Need individual port for each pair of devices
#define GUIDANCE_TCP_PORT       38000
#define NAVIGATION_TCP_PORT     38010
#define CONTROL_TCP_PORT        38020
#define DRAGONBOARD_TCP_PORT    38030

#endif // MR_FUSION_SYSTEM_CONFIG

