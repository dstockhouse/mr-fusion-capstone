
#ifndef __CRC_H
#define __CRC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#define X25_INIT_CRC 0xffff
#define X25_VALIDATE_CRC 0xf0b8
#define CRC_EXTRA 184


void crc_accumulate(uint8_t data, uint16_t *crcAccum);

void crc_init(uint16_t *crcAccum);

uint16_t crc_calculate(const uint8_t *pBuffer, uint16_t length);

void crc_accumulate_buffer(uint16_t *crcAccum, const char *pBuffer, uint16_t length);

#endif

