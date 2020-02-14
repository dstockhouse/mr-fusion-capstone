/****************************************************************************
 *
 * File:
 * 	buftest.c
 *
 * Description:
 * 	Tests the buffer functionality
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/13/2019
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "debuglog.h"

#include "buffer.h"

#define DATA_LEN 20000
#define REMOVE_LEN 6000


/**** Function main ****
 *
 * Test of the buffer module functionality
 *
 * Arguments: None
 *
 * Return value:
 * 	Returns 0 on success
 * 	Returns a negative number on failure
 */
int main(void) {

    int i;

    BYTE_BUFFER testbuf;
    unsigned char testval = 200;
    unsigned char data[DATA_LEN];

    // Fill data array
    for(i = 0; i < DATA_LEN; i++) {
        data[i] = i;
    }

    // Initialize
    BufferEmpty(&testbuf);

    logDebug("Adding %d...\n", testval);

    BufferAdd(&testbuf, testval);

    logDebug("\tNew size: %d\n\t", testbuf.length);

//     for(i = 0; i < BufferLength(&testbuf); i++) {
//         logDebug("%d, ", BufferIndex(&testbuf, i));
//         if(i % 20 == 19) logDebug("\n\t");
//     }

    logDebug("\n\nAdding array...\n");

    BufferAddArray(&testbuf, data, DATA_LEN);

    logDebug("\tNew size: %d\n\t", testbuf.length);

//     for(i = 0; i < BufferLength(&testbuf); i++) {
//         logDebug("%d, ", BufferIndex(&testbuf, i));
//         if(i % 20 == 19) logDebug("\n\t");
//     }

    while (BufferLength(&testbuf) > 0) {
        logDebug("\n\nRemoving (%d) elements...\n", REMOVE_LEN);

        int removed = BufferRemove(&testbuf, REMOVE_LEN);
        logDebug("%d removed\n", removed);

        logDebug("\tNew size: %d\n\t", testbuf.length);

//     for(i = 0; i < BufferLength(&testbuf); i++) {
//         logDebug("%d, ", BufferIndex(&testbuf, i));
//         if(i % 20 == 19) logDebug("\n\t");
//     }
    }

    logDebug("\n\nRemoving too many (%d)...\n", testbuf.length + 30);

    i = BufferRemove(&testbuf, testbuf.length);

    logDebug("\tNew size: %d\n\n\n", testbuf.length);

//     for(i = 0; i < BufferLength(&testbuf); i++) {
//         logDebug("%d, ", BufferIndex(&testbuf, i));
//         if(i % 20 == 19) logDebug("\n\t");
//     }


    // Now put it through a lot of random adding and deleting

    // Seed rand()
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    srand(time.tv_sec + time.tv_nsec);

    const unsigned int maxAdd = 18000;
    unsigned char randdata[maxAdd+1];
    int iters = 20000000;
    for (i = 0; i < iters; i++) {

        logDebug(" %3d:  ", i);

        int numToAdd = rand() % maxAdd;

        logDebug("populating %d... ", numToAdd);

        int j;
        for (j = 0; j < numToAdd; j++) {
            randdata[i] = numToAdd + j;
        }
        logDebug("Adding (%d)... ", numToAdd);
        int added = BufferAddArray(&testbuf, randdata, numToAdd);
        logDebug("added %d, length (%d), s,e=(%d,%d)\n", added, BufferLength(&testbuf), testbuf.start, testbuf.end);

        logDebug("       ");

        int numToRemove = rand() % 20000;

        logDebug("Removing (%d)... ", numToRemove);
        int removed = BufferRemove(&testbuf, numToRemove);
        logDebug("removed %d, length (%d), s,e=(%d,%d)\n", removed, BufferLength(&testbuf), testbuf.start, testbuf.end);

    }

    logDebug("\n\nTest complete\n\n");

    // Success
    return 0;

} // main()

