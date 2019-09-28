/****************************************************************************\
 *
 * File:
 * 	logger.h
 *
 * Description:
 * 	Function and type declarations and constants for logger.c
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/20/2019
 *
\***************************************************************************/

#ifndef __LOGGER_H
#define __LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// To use localtime_r from generateFilename
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define LOG_FILENAME_LENGTH 256

// Extension type identifiers
#define LOG_FILEEXT_LOG 0
#define LOG_FILEEXT_BIN 1
#define LOG_FILEEXT_CSV 2

typedef struct {
	int fd;
	int bin;
	char filename[LOG_FILENAME_LENGTH];
	int filenameLength;
	time_t timestamp;
} LOG_FILE;

int generateFilename(char *buf, int bufSize, time_t *time, 
		const char *dir, const char *pre, const char *ext);

int LogInit(LOG_FILE *logFile, const char *dir, const char *pre, int ext);

int LogUpdate(LOG_FILE *logFile, const char *buf, int length);

int LogClose(LOG_FILE *logFile);

#endif

