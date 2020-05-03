/****************************************************************************
 *
 * File:
 * 	encoder.h
 *
 * Description:
 * 	Function and type declarations and constants for encoder.c
 *
 * Author:
 * 	Duncan Patel
 *
 * Revision 0.1
 * 	Last edited 03/05/2020
 *
 ***************************************************************************/

#ifndef __ENCODER_H
#define __ENCODER_H

typedef struct {
    int fd;
} ENCODER_DEV;

int EncoderInit(ENCODER_DEV *dev);

#endif // __ENCODER_H

