/****************************************************************************\
 *
 * File:
 * 	fifo.h
 *
 * Description:
 * 	Header files and function headers for the FIFO module
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/13/2019
 *
\***************************************************************************/

#define UART_DEV "/dev/ttyUSB0"

#define FIFO_LEN (16384)

typedef struct {
	int size;
	int start;
	int end;
	unsigned char buffer[FIFO_LEN];
} BYTE_FIFO;

int FIFOAdd(BYTE_FIFO *fifo, unsigned char data) {

}

