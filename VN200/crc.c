/****************************************************************************\
 *
 * File:
 * 	crc.c
 *
 * Description:
 * 	Checksum and CRC for VN200 IMU/GPS
 *
 * Author:
 * 	Adapted from VN200 User Manual
 *
 * Revision 0.1
 * 	Last edited 4/16/2019
 *
\***************************************************************************/

#include "crc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

// Calculates the 8-bit checksum for the given byte sequence. 
unsigned char calculateChecksum(unsigned char data[], unsigned int length) {

	unsigned int i;
	unsigned char cksum = 0;

	// printf("\t\t\tIn calculateChecksum:\n");
	for(i=0; i<length; i++){
		cksum ^= data[i];
		// printf("\t\t\t        Adding %c; %02x\n", data[i], cksum);
	}

	return cksum;
}

// Calculates the 16-bit CRC for the given ASCII or binary message.
unsigned short calculateCRC(unsigned char data[], unsigned int length) {

	unsigned int i;
	unsigned short crc = 0;

	for(i=0; i<length; i++){
		crc  = (unsigned char)(crc >> 8) | (crc << 8);
		crc ^= data[i];
		crc ^= (unsigned char)(crc & 0xff) >> 4;
		crc ^= crc << 12;
		crc ^= (crc & 0x00ff) << 5;
	}

	return crc;
}

