/****************************************************************************
 *
 * File:
 * 	kangaroo.c
 *
 * Description:
 * 	Hardware abstraction for interfacing with the UART Kangaroo Motion Controller
 *
 * Author:
 * 	Duncan Patel
 *
 * Revision 0.1
 * 	Last edited 03/05/2020
 *
 ***************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "debuglog.h"

#include "kangaroo.h"

/**** Function KangarooInit ****
 *
 * 
 *
 * Arguments: 
 * 	dev - Pointer to Kangaroo object to initialize
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int KangarooInit(KANGAROO_DEV *dev) {

    if (dev == NULL) {
        return -1;
    }

    return 0;

} // KangarooInit(KANGAROO_DEV *)

