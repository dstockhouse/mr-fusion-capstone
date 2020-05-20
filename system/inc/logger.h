/****************************************************************************
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
 * 	Last edited 02/20/2019
 *
 * Revision 0.2
 * 	Last edited 02/13/2020
 *
 * Revision 0.3
 * 	Last edited 05/18/2020
 * 	Moved generateFilename and mkdir_p to utils
 * 	Adapted to more general definition for mkdir_p
 *
 ***************************************************************************/

#ifndef __LOGGER_H
#define __LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

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

int LogInit(LOG_FILE *logFile, const char *dir, const char *pre,
        time_t *logtime, unsigned key, int ext);

int LogUpdate(LOG_FILE *logFile, const char *buf, int length);

int LogFlush(LOG_FILE *logFile);

int LogClose(LOG_FILE *logFile);

#endif

