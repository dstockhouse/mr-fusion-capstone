/****************************************************************************\
 *
 * File:
 * 	adsb_crc.h
 *
 * Description:
 * 	Function and type declarations and constants for crc.c
 *
 * Author:
 * 	Adapted from uAvionix pingUSB integration guide
 *
 * Revision 0.1
 * 	Last edited 2/28/2019
 *
\***************************************************************************/

#ifndef __ADSB_CRC_H
#define __ADSB_CRC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

// For traffic report packet
#define ADSB_CRC_EXTRA 184
#define ADSB_X25_INIT_CRC 0xffff


void crc_accumulate(uint8_t data, uint16_t *crcAccum);

void crc_init(uint16_t *crcAccum);

uint16_t crc_calculate(const uint8_t *pBuffer, uint16_t length);

void crc_accumulate_buffer(uint16_t *crcAccum, const char *pBuffer, uint16_t length);

#endif

