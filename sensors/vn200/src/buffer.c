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
 * 	Changed into a ring buffer implementation (all under the hood)
 * 	Code that uses buffer.length may need to be modified
 * 	Last edited 2/13/2020
 *
 \***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include "debuglog.h"

#include "buffer.h"

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
int BufferAdd(BYTE_BUFFER *buf, unsigned char data) {

    // Exit if buffer pointer invalid
    if (buf == NULL) {
        return -1;
    }

    // Ensure buffer is not full
    if (!BufferIsFull(buf)) {

        // Increase length and put element at end of buffer
        buf->buffer[buf->end] = data;
        buf->end = BYTE_BUFFER_MOD(buf->end + 1);
        buf->length = BufferLength(buf);

        // Added one element
        return 1;

    }

    // Didn't add anything
    return 0;

} // BufferAdd(BYTE_BUFFER *, unsigned char)


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
int BufferAddArray(BYTE_BUFFER *buf, unsigned char *data, int numToAdd) {

    int i, numAdded = 0;

    if (buf == NULL) {
        return -1;
    }

    // Add as many elements as will fit
    for (i = 0; i < numToAdd && !BufferIsFull(buf); i++) {

        // logDebug(L_VDEBUG, "\nAdded elt %d, l=%d", i, BufferLength(buf));

        // Use above function to add one element and increment if successful
        if (BufferAdd(buf, data[i]) > 0) {
            numAdded++;
        }
    }

    // Return number successfully added
    return numAdded;

} // BufferAddArray(BYTE_BUFFER *, unsigned char *, int)


/**** Function BufferRemove ****
 *
 * Removes a number of values from the start of a BYTE_BUFFER instance
 *
 * Arguments: 
 * 	buf         - Pointer to BYTE_BUFFER instance to modify
 * 	numToRemove - Number of elements to remove from buffer
 *
 * Return value:
 * 	On returns number of elements removed. Zero elements returned on error.
 */
int BufferRemove(BYTE_BUFFER *buf, int numToRemove) {

    if (buf == NULL || numToRemove <= 0) {
        return 0;
    }

    // If requesting to remove more than the length, just remove the length
    if (numToRemove > BufferLength(buf)) {
        numToRemove = BufferLength(buf);
    }

    // Remove from start by changing start index (don't shift elements)
    buf->start = BYTE_BUFFER_MOD(buf->start + numToRemove);

    // Update new length with the number of elements removed (how far the loop made it)
    buf->length = BufferLength(buf);

    // Return number actually removed (either num requested or previous length)
    return numToRemove;

} // BufferRemove(BYTE_BUFFER *, int)


/**** Function BufferIndex ****
 *
 * Accesses an element at an index. Invalid indices still return something
 * indistinguishable from valid data, so it is up to the calling function to
 * make sure the index is less than the buffer length.
 *
 * Arguments: 
 * 	buf   - Pointer to BYTE_BUFFER instance to modify
 * 	index - Offset from start to index
 *
 * Return value:
 * 	On returns number of elements removed. Zero elements returned on error.
 */
unsigned char BufferIndex(BYTE_BUFFER *buf, int index) {

    if (buf == NULL || index < 0 || index >= BufferLength(buf)) {
        // Failure can't be detected by the calling function, but at least don't crash
        // logDebug(L_VDEBUG, "Attempt to index past buffer bounds: l=%d, i=%d\n", BufferLength(buf), index);
        return 0;
    }

    // Return number at that index
    return buf->buffer[BYTE_BUFFER_MOD(buf->start + index)];

} // BufferIndex(BYTE_BUFFER *, int)


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
    if (buf == NULL) {
        // Invalid buffer is treated as full
        return -1;
    }

    // Change indices to ignore all elements previously stored, and rezero
    buf->start = buf->end = 0;
    buf->length = 0;

    // Return on success
    return 0;

} // BufferEmpty(BYTE_BUFFER *)


/**** Function BufferIsFull ****
 *
 * Returns true if the buffer is full
 *
 * Arguments: 
 * 	buf - Pointer to BYTE_BUFFER instance to examine
 *
 * Return value:
 * 	On success returns 0 if empty, 1 if full
 */
int BufferIsFull(BYTE_BUFFER *buf) {

    // Exit if buffer pointer invalid
    if (buf == NULL) {
        // Invalid buffer is treated as full
        return -1;
    }

    // Return true if start + 1 == end
    return (BufferLength(buf) == (BYTE_BUFFER_LEN - 1));

} // BufferIsFull(BYTE_BUFFER *)


/**** Function BufferLength ****
 *
 * Returns number of elements currently stored in circular buffer
 *
 * Arguments: 
 * 	buf - Pointer to BYTE_BUFFER instance to examine
 *
 * Return value:
 * 	On success returns 0 if empty, 1 if full
 */
int BufferLength(BYTE_BUFFER *buf) {

    // Exit if buffer pointer invalid
    if (buf == NULL) {
        // Invalid buffer
        return -1;
    }

    // Return true if start + 1 == end
    return BYTE_BUFFER_MOD(buf->end - buf->start);

} // BufferLength(BYTE_BUFFER *)


/**** Function BufferCopy ****
 *
 * Copies elements from the circular buffer into a linear array
 *
 * Arguments: 
 * 	buf   - Pointer to BYTE_BUFFER instance to copy from
 * 	dest  - Pointer to linear destination buffer
 * 	start - Index in buf to start copying
 * 	num   - Number of elements to copy
 *
 * Return value:
 * 	On success returns number of elements copied
 */
int BufferCopy(BYTE_BUFFER *buf, unsigned char *dest, int start, int num) {

    // Exit if buffer pointer invalid
    if (buf == NULL || dest == NULL) {
        // Invalid buffer
        return -1;
    }

    // Copy no elements if inputs are negative
    if (start < 0 || num < 0) {
        return 0;
    }

    // If less elements in buffer than requested, adjust number to copy
    int numAvailable = BufferLength(buf) - start;
    if (numAvailable < num) {
        num = numAvailable;
    }

    // Copy buffer elements into destination buffer starting at start index
    int i;
    for (i = 0; i < num; i++) {
        dest[i] = BufferIndex(buf, i + start);
    }

    return i;

} // BufferCopy(BYTE_BUFFER *, unsigned char *, int, int)

