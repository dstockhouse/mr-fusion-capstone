/****************************************************************************\
 *
 * File:
 * 	buffer.h
 *
 * Description:
 * 	Function and type declarations and constants for buffer.c
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/20/2019
 *
 * Revision 0.2
 * 	Last edited 2/13/2020
 *
 \***************************************************************************/

#ifndef __BUFFER_H
#define __BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include "utils.h"

// 16K buffer, for now
#define BYTE_BUFFER_LEN (16384)

#define BYTE_BUFFER_MOD(T) MOD(T, BYTE_BUFFER_LEN)

typedef struct {
    int start, end, length;
    unsigned char buffer[BYTE_BUFFER_LEN];
} BYTE_BUFFER;

int BufferAdd(BYTE_BUFFER *buf, unsigned char data);

int BufferAddArray(BYTE_BUFFER *buf, unsigned char *data, int length);

int BufferRemove(BYTE_BUFFER *buf, int numToRemove);

unsigned char BufferIndex(BYTE_BUFFER *buf, int index);

int BufferEmpty(BYTE_BUFFER *buf);

int BufferIsFull(BYTE_BUFFER *buf);

int BufferLength(BYTE_BUFFER *buf);

#endif

