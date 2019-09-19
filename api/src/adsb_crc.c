/****************************************************************************\
 *
 * File:
 * 	adsb_crc.c
 *
 * Description:
 * 	X.25 checksum calculation for MAVLink packets
 *
 * Author:
 * 	Adapted from uAvionix pingUSB integration guide
 *
 * Revision 0.1
 * 	Last edited 2/28/2019
 *
\***************************************************************************/

#include "adsb_crc.h"

/**
 * @brief Accumulate the X.25 CRC by adding one char at a time.
 *
 * The checksum function adds the hash of one char at a time to the
 * 16 bit checksum (uint16_t).
 *
 * @param data - New char to hash
 * @param crcAccum - Already accumulated checksum
 **/
void crc_accumulate(uint8_t data, uint16_t *crcAccum)
{
	// Accumulate one byte of data into the CRC
	uint8_t tmp;
	tmp = data ^ (uint8_t)(*crcAccum&0xff);
	tmp ^= (tmp<<4);
	*crcAccum = (*crcAccum>>8) ^ (tmp<<8) ^ (tmp<<3) ^ (tmp>>4);
}

/**
 * @brief Initialize the buffer for the X.25 CRC
 *
 * @param crcAccum - 16 bit X.25 CRC
 */
void crc_init(uint16_t *crcAccum)
{
	*crcAccum = ADSB_X25_INIT_CRC;
}

/**
 * @brief Calculates the X.25 checksum on a byte buffer
 *
 * @param pBuffer - buffer containing the byte array to hash
 * @param length - length of the byte array
 * @return the checksum over the buffer bytes
 **/
uint16_t crc_calculate(const uint8_t *pBuffer, uint16_t length)
{
	uint16_t crcTmp;
	crc_init(&crcTmp);
	while (length--) crc_accumulate(*pBuffer++, &crcTmp);
	crc_accumulate(ADSB_CRC_EXTRA, &crcTmp);
	return crcTmp;
}

/**
 * @brief Accumulate the X.25 CRC by adding an array of bytes
 *
 * The checksum function adds the hash of one char at a time to the
 * 16 bit checksum (uint16_t).
 *
 * @param data - New bytes to hash
 * @param crcAccum - Already accumulated checksum
 **/
void crc_accumulate_buffer(uint16_t *crcAccum, const char *pBuffer, uint16_t length)
{
	const uint8_t *p = (const uint8_t *)pBuffer;
	while (length--) crc_accumulate(*p++, crcAccum);
}

