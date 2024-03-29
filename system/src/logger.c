/****************************************************************************
 *
 * File:
 * 	logger.c
 *
 * Description:
 * 	Sets up a binary or text log file timestamped at open
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/20/2019
 *
 * Revision 0.2
 * 	Added function to flush log file
 * 	Last edited 2/13/2020
 *
 ***************************************************************************/

#include "debuglog.h"

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

/**** Function generateFilename ****
 *
 * Generate a timestamped filename that matches the format 
 * "pre_mm.dd.yyyy_hh-mm-ss.ext" in the directory "dir"
 *
 * Arguments:
 * 	buf     - String buffer to hold generated string
 * 	bufSize - Length of buffer, will not write past this length
 * 	dir     - String directory name to include in filename. Must end with /
 * 	pre     - Prefix to put at start of new filename
 * 	ext     - Extension to put at end of new filename
 *
 * Return value:
 * 	Returns the number of characters written to buf (length of new string)
 */
int generateFilename(char *buf, int bufSize, time_t *filetime, 
        const char *dir, const char *pre, const char *ext) {

    // Length of filename generated
    int charsWritten;

    struct timespec randseed;
    // Different time source
    // clock_gettime(CLOCK_MONOTONIC_RAW, &randseed);
    clock_gettime(CLOCK_MONOTONIC, &randseed);
    srand(randseed.tv_sec + randseed.tv_nsec);

    // Time variables
    struct tm currentTime;
    time_t ltime;
    if(filetime == NULL) {
        ltime = time(NULL);
        filetime = &ltime;
    }

    // Get current time in UTC
    localtime_r(filetime, &currentTime);

    // Create filename using date/time and input string
    charsWritten = snprintf(buf, bufSize, 
            "%s/%s-%02d.%02d.%04d_%02d-%02d-%02d_%d.%s",
            dir, pre,
            currentTime.tm_mon + 1,
            currentTime.tm_mday,
            currentTime.tm_year + 1900,
            currentTime.tm_hour,
            currentTime.tm_min,
            currentTime.tm_sec,
            rand(),
            ext);

    // Return length of the new string
    return charsWritten;

} // generateFilename(char *, int, time_t, char *, char *, char *)


/**** Function mkdir_p ****
 *
 * Implementation of mkdir -p using system service calls, adapted from
 * https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950
 *
 * Arguments:
 * 	pathname - String path to the directory to create
 * 	mode     - Directory access mode (permissions) for mkdir
 *
 * Return value:
 * 	On success, returns 0, otherwise returns -1 and sets errno
 */
int mkdir_p(const char *pathname, mode_t mode) {

    int pathnamelen, rc;
    char localpathname[PATH_MAX], *dir;

    // Ensure pathname length is small enough
    pathnamelen = strlen(pathname);
    if(pathnamelen > PATH_MAX - 1) {
        errno = ENAMETOOLONG;
        return -1;
    }

    // Copy to local string to allow (temp) modifications
    strcpy(localpathname, pathname);

    for(dir = localpathname + 1; *dir != '\0'; dir++) {

        // If end of directory, mkdir everything before this
        if(*dir == '/') {

            // Temporarily terminate string here
            *dir = '\0';

            rc = mkdir(localpathname, mode);
            if(rc && errno != EEXIST) {
                return rc;
            }

            // Restore
            *dir = '/';
        }

    } // for

    // Make final directory
    rc = mkdir(pathname, mode);
    if(rc && errno != EEXIST) {
        return rc;
    }

    return 0;

} // int mkdir_p(const char *, mode_t)


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
int LogInit(LOG_FILE *logFile, const char *dir, const char *pre, int ext) {

    int rc;
    char extString[8];

    // Get seconds since epoch
    logFile->timestamp = time(NULL);

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
            &(logFile->timestamp), dir, pre, extString);
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

} // LogInit(LOG_FILE *, char *, char *, int)


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

