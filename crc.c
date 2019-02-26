#define X25_INIT_CRC 0xffff
#define X25_VALIDATE_CRC 0xf0b8

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
	*crcAccum = X25_INIT_CRC;
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


// Note CRC_EXTRA is defined for each individual packet in the document.
// crc_accumulate_buffer(&msg->checksum, _PAYLOAD(msg), msg->len);
// crc_accumulate(CRC_EXTRA, &msg->checksum);
// ck_a(msg) = (uint8_t)(msg->checksum & 0xFF);
// ck_b(msg) = (uint8_t)(msg->checksum >> 8);
