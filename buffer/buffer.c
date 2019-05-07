/****************************************************************************\
 *
 * File:
 * 	buffer.c
 *
 * Description:
 * 	Queue-like buffer for storing data that is read in any order but 
 * 	added and removed in FIFO order. It is not a ring buffer, data always
 * 	starts at index 0 until length-1
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/20/2019
 *
 * Revision 0.2
 * 	Changed return value of BufferRemove
 * 	Last edited 2/25/2019
 *
\***************************************************************************/

#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>


/**** Function BufferAdd ****
 *
 * Adds a value to a BYTE_BUFFER instance
 *
 * Arguments: 
 * 	buf  - Pointer to BYTE_BUFFER instance to modify
 * 	data - Single byte to add to array
 *
 * Return value:
 * 	Returns number of elements added (1 or 0)
 *      If buf is NULL returns -1
 */
int BufferAdd(BYTE_BUFFER *buf, char data) {

	// Exit if buffer pointer invalid
	if(buf == NULL) {
		return -1;
	}

	// Ensure buffer is not full
	if(buf->length < BYTE_BUFFER_LEN - 1) {

		// Increase length and put element at end of buffer
		buf->buffer[buf->length] = data;
		buf->length += 1;

		// Added one element
		return 1;

	} // else

	// Didn't add anything
	return 0;

} // BufferAdd(BYTE_BUFFER *, char)


/**** Function BufferAddArray ****
 *
 * Adds a supplied array of values to a BYTE_BUFFER instance
 *
 * Arguments: 
 * 	buf      - Pointer to BYTE_BUFFER instance to modify
 * 	data     - Array containing data to add to buf
 * 	numToAdd - Number of elements to add to buffer
 *
 * Return value:
 * 	On success returns number of elements successfully added
 *      If buf is NULL returns -1
 */
int BufferAddArray(BYTE_BUFFER *buf, char *data, int numToAdd) {

	int i, numAdded = 0;

	if(buf == NULL) {
		return -1;
	}

	// Add as many elements as will fit
	for(i = 0; i < numToAdd; i++) {

		// Use above function to add one element and increment if successful
		if(BufferAdd(buf, data[i]) > 0) {
			numAdded++;
		}
	}

	// Return number successfully added
	return numAdded;

} // BufferAddArray(BYTE_BUFFER *, char *, int)


/**** Function BufferRemove ****
 *
 * Removes a number of values from the start of a BYTE_BUFFER instance
 *
 * Arguments: 
 * 	buf         - Pointer to BYTE_BUFFER instance to modify
 * 	numToRemove - Number of elements to remove from buffer
 *
 * Return value:
 * 	On success returns number of elements removed
 *      If buf is NULL returns -1
 */
int BufferRemove(BYTE_BUFFER *buf, int numToRemove) {

	int i, numRemoved;

	if(buf == NULL) {
		return -1;
	}

	if(numToRemove <= 0) {
		return 0;
	}

	// Always remove from index 0 and shift elements backward
	for(i = 0; i + numToRemove < buf->length; i++) {
		buf->buffer[i] = buf->buffer[i + numToRemove];
	}

	// Calculate number of elements that were actually removed
	numRemoved = buf->length - i;

	// Update new length with the number of elements removed (how far the loop made it)
	buf->length = i;

	// Return number actually removed
	return numRemoved;

} // BufferRemove(BYTE_BUFFER *, int)


/**** Function BufferEmpty ****
 *
 * Empties a BYTE_BUFFER instance
 *
 * Arguments: 
 * 	buf - Pointer to BYTE_BUFFER instance to empty
 *
 * Return value:
 * 	On success returns 0
 *      If buf is NULL returns -1
 */
int BufferEmpty(BYTE_BUFFER *buf) {

	// Exit if buffer pointer invalid
	if(buf == NULL) {
		// Invalid buffer is treated as full
		return -1;
	}

	// Leave data in buffer but consider data to be invalid
	buf->length = 0;

	// Return on success
	return 0;

} // BufferEmpty(BYTE_BUFFER *)

