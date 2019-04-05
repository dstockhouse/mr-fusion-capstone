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
\***************************************************************************/

#ifndef __BUFFER_H
#define __BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

// 16K buffer, for now
#define BYTE_BUFFER_LEN (16384)

typedef struct {
	int length;
	int start;
	int end;
	char buffer[BYTE_BUFFER_LEN];
} BYTE_BUFFER;

int BufferAdd(BYTE_BUFFER *buf, char data);

int BufferAddArray(BYTE_BUFFER *buf, char *data, int length);

int BufferRemove(BYTE_BUFFER *buf, int numToRemove);

int BufferEmpty(BYTE_BUFFER *buf);

#endif

