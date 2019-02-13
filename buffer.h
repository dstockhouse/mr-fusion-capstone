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

#define FIFO_LEN (16384)

typedef struct {
	int size;
	int start;
	int end;
	unsigned char buffer[FIFO_LEN];
} BYTE_BUFFER;

int FIFOAdd(BYTE_BUFFER *fifo, unsigned char data);

int FIFOAddArray(BYTE_BUFFER *fifo, unsigned char *data, int length);

