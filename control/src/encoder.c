/****************************************************************************
 *
 * File:
 * 	encoder.c
 *
 * Description:
 * 	Wrapper for SPI interfacing to the encoder buffer
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

#include "utils.h"

#include "encoder.h"

/**** Function EncoderInit ****
 *
 * Initializes encoder object
 *
 * Arguments: 
 * 	relativeHeading - Desired heading change for controller input (radians)
 *
 * Return value:
 * 	On success, returns 0
 *	On failure, returns a negative number 
 */
int EncoderInit(ENCODER_DEV *dev) {

    if (dev == NULL) {
        return -1;
    }

    return 0;

} // EncoderInit(ENCODER_DEV *)

