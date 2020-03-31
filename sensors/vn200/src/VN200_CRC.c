/****************************************************************************\
 *
 * File:
 * 	VN200_CRC.c
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

#include "debuglog.h"

#include "VN200_CRC.h"

// Calculates the 8-bit checksum for the given byte sequence. 
unsigned char VN200CalculateChecksum(unsigned char data[], unsigned int length) {

    unsigned int i;
    unsigned char cksum = 0;

    // Keeping these log statements here as they are helpful debugging CRC errors
    // logDebug(L_VDEBUG, "\t\t\tIn calculateChecksum:\n");
    for(i=0; i<length; i++){
        cksum ^= data[i];
        // logDebug(L_VDEBUG, "\t\t\t        Adding %c; %02x\n", data[i], cksum);
    }

    return cksum;
}

// Calculates the 16-bit CRC for the given ASCII or binary message.
unsigned short VN200CalculateCRC(unsigned char data[], unsigned int length) {

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

