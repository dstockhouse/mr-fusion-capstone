/****************************************************************************
 *
 * File:
 * 	logger.c
 *
 * Description:
 * 	Utilities to create and manage a log file in a consistent manner.
 * 	These functions are wrappers around POSIX basic file I/O
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 02/20/2019
 *
 * Revision 0.2
 * 	Added function to flush log file
 * 	Last edited 02/13/2020
 *
 * Revision 0.3
 * 	Last edited 05/18/2020
 * 	Moved generateFilename and mkdir_p to utils
 * 	Adapted to more general definition for mkdir_p
 *
 ***************************************************************************/

#include "config.h"
#include "utils.h"

#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>


/**** Function LogInit ****
 *
 * Creates a timestamp and log file
 *
 * Arguments:
 * 	logFile - Pointer to LOG object to initialize
 * 	dir     - String name of the directory to create the log file in
 * 	pre     - String prefix to use for the log file
 * 	ext     - Extension ID, see logger.h for LOG_FILEEXT_* constants
 *
 * Return value:
 * 	On success, returns 0, otherwise returns a negative number
 */
int LogInit(LOG_FILE *logFile, const char *dir, const char *pre, time_t *logtime, unsigned key, int ext) {

    int rc;
    char extString[8];

    // Get seconds since epoch
    time_t tempTime;
    if (logtime == NULL) {
        tempTime = time(NULL);
        logtime = &tempTime;
    }
    logFile->timestamp = *logtime;

    // Determine filename extension, default is log
    switch(ext) {
        case LOG_FILEEXT_BIN:
            strcpy(extString, "bin");
            logFile->bin = 1;
            break;

        case LOG_FILEEXT_CSV:
            strcpy(extString, "csv");
            logFile->bin = 0;
            break;

        case LOG_FILEEXT_LOG:
        default:
            strcpy(extString, "log");
            logFile->bin = 0;
            break;

    } // switch(ext)

    // Generate filename for the log file
    logFile->filenameLength = generateFilename(logFile->filename, LOG_FILENAME_LENGTH, 
            &(logFile->timestamp), dir, pre, key, extString);
    if(logFile->filenameLength == LOG_FILENAME_LENGTH) {
        logDebug(L_INFO, "Filename too long, using %s\n", logFile->filename);
    }

    // Create directory if it doesn't exist
    rc = mkdir_p(dir, 0777);
    // rc = mkdir(dir, 0777);
    if(rc && errno != EEXIST) {
        logDebug(L_INFO, "%s: Failed to create directory '%s'\n", strerror(errno), dir);
        return -1;
    }

    // Open and create the log file, appending if it already exists
    logFile->fd = open(logFile->filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if(logFile->fd < 0) {
        logDebug(L_INFO, "%s: Failed to create log file %s\n", strerror(errno), logFile->filename);
        return logFile->fd;
    }
    logDebug(L_INFO, "Created log file %s\n", logFile->filename);

    // Flush file to ensure creation
    LogFlush(logFile);

    // Return 0 on success
    return 0;

} // LogInit(LOG_FILE *, char *, char *, time_t *, unsigned, int)


/**** Function LogUpdate ****
 *
 * Writes a set of bytes to an existing log file
 *
 * Arguments:
 * 	logFile - Pointer to LOG object to update
 * 	buf     - Bytes to write to log
 * 	length  - Number of bytes in the buffer to the log file
 *
 * Return value:
 * 	On success, returns number of characters written, otherwise returns a 
 * 	negative number
 */
int LogUpdate(LOG_FILE *logFile, const char *buf, int length) {

    int rc;

    // printf("In LU: attempting to read %d bytes from address %p\n", length, buf);

    // Write data to file
    rc = write(logFile->fd, buf, length);
    if(rc < 0) {
        logDebug(L_INFO, "%s: Failed to write to log file\n", strerror(errno));
    }

    // Flush always for now, but may change later
    // LogFlush(logFile);

    // Return bytes written
    return rc;

} // LogUpdate(LOG_FILE *, char *, int)


/**** Function LogFlush ****
 *
 * Flushes all pending I/O writes to disk
 *
 * Arguments:
 * 	logFile - The logger object to flush
 *
 * Return value:
 * 	On success, returns 0, otherwise returns an error code
 */
int LogFlush(LOG_FILE *logFile) {

    int rc;

    rc = fsync(logFile->fd);
    if(rc) {
        logDebug(L_INFO, "%s: Failed to sync log file\n", strerror(errno));
    }

    // Return code from system service call
    return rc;

} // LogFlush(LOG_FILE *)


/**** Function LogClose ****
 *
 * Closes an open log file, which can no longer be used
 *
 * Arguments:
 * 	logFile - The logger object to close
 *
 * Return value:
 * 	On success, returns 0, otherwise returns a negative number
 */
int LogClose(LOG_FILE *logFile) {

    int rc;

    rc = close(logFile->fd);
    if(rc) {
        logDebug(L_INFO, "%s: Failed to close log file\n", strerror(errno));
    }

    // Return code from system service call
    return rc;

} // LogClose(LOG_FILE *)

