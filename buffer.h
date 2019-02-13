/****************************************************************************\
 *
 * File:
 * 	buffer.h
 *
 * Description:
 * 	Header files and function headers for the buffer
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/13/2019
 *
\***************************************************************************/

#define BYTE_BUFFER_LEN (16384)

typedef struct {
	int length;
	int end;
	unsigned char buffer[BYTE_BUFFER_LEN];
} BYTE_BUFFER;

int BufferAdd(BYTE_BUFFER *buf, unsigned char data);

int BufferAddArray(BYTE_BUFFER *buf, unsigned char *data, int length);

int BufferRemove(BYTE_BUFFER *buf, int numToRemove);

