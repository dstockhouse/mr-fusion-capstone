/****************************************************************************\
 *
 * File:
 * 	crc.h
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

#ifndef __CRC_H
#define __CRC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

unsigned char calculateChecksum(unsigned char data[], unsigned int length);

unsigned short calculateCRC(unsigned char data[], unsigned int length);

#endif

