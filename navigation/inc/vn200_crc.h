/****************************************************************************\
 *
 * File:
 * 	vn200_crc.h
 *
 * Description:
 * 	Function and type declarations and constants for crc.c
 *
 * Author:
 * 	Adapted from VN200 User Manual
 *
 * Revision 0.1
 * 	Last edited 4/16/2019
 *
\***************************************************************************/

#ifndef __VN200_CRC_H
#define __VN200_CRC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

unsigned char VN200CalculateChecksum(unsigned char data[], unsigned int length);

/* From VN200 documentation. Used for binary messages, so keeping around in case
unsigned short VN200CalculateCRC(unsigned char data[], unsigned int length);
*/

#endif

