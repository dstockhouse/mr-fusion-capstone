/***************************************************************************\
 *
 * File:
 * 	utils.c
 *
 * Description:
 *	Simple utilities used in the system
 * 
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 05/17/2020
 *
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"
#include "logger.h"

#include "utils.h"


/**** Function getTimestamp ****
 *
 * Gets a system timestamp as a double and struct timespec
 * Only one of the arguments need not be null
 *
 * Arguments: 
 * 	ts - Pointer to struct timespec to be used in gettime function
 * 	td - Pointer to double to store processed timestamp
 *
 * Return value:
 *	On success, returns 0
 *	On failure, returns a negative number
 */
int getTimestamp(struct timespec *ts, double *td) {

    int rc;

    // Used by gettime if input timespec pointer is null
    struct timespec lts;

    if (ts == NULL && td == NULL) {
        return -2;
    }

    // Set to local timespec if input is NULL
    if (ts == NULL) {
        ts = &lts;
    }

    // Get system time
    rc = clock_gettime(CLOCK_REALTIME, ts);
    if (rc) {
        return rc;
    }

    // Return (by reference) time as double
    if (td != NULL) {
        *td = ((double) ts->tv_sec) + ((double) ts->tv_nsec) / 1000000000;
    }

    return 0;

} // getTimestamp(struct timespec *, double *)


/**** Function logDebug ****
 *
 * The function's definition depends on DEBUG_OUT_**** set in config.h at
 * compile time
 *
 * 1-3 options can be defined individually or simultaneously, in which case
 * debug info will be logged in more than one location
 *
 * Arguments: 
 * 	debuglevel - Integer level of severity for the log
 * 	fstring    - Pointer to string to be formatted
 * 	...        - Formatting arguments to be used in the format string
 *
 * Return value:
 *	Does not return a value
 */
void logDebug(int debuglevel, const char *fstring, ...) {

    // Only log anything if the level is higher than the allowed mask
    if (debuglevel >= CONTROL_DEBUG_L_MASK) {

        // Variadic arguments to pass to printf
        va_list args;

        // For each logging source, variadic arguments must be started and ended

#ifdef DEBUG_OUT_SYSLOG

        // Start variadic argument traversal
        va_start(args, fstring);

        // Initialize if this is the first time calling
        static int syslogInitialized = 0;
        if(!syslogInitialized) {
            syslogInitialized = 1;

            // Initialize syslog for the program.
            // Every log message starts with "ICARUS_HFNAV"
            // PID included in log, errors logged to console
            // Logged at user level
            // Mask only DEBUG log messages
            char syslogID[64];
            srand(time(NULL));
            snprintf(syslogID, 64, "ICARUS_HFNAV_%d", rand());
            openlog(syslogID, LOG_PID | LOG_CONS, LOG_USER);
            setlogmask(LOG_MASK(LOG_DEBUG));

            printf("Logging to syslog under ID %s\n", syslogID);
        }

        // Format string to output
        vsyslog(LOG_DEBUG, fstring, args);

        // End variadic arguments
        va_end(args);

#endif
#ifdef DEBUG_OUT_LOGFILE

        // Start variadic argument traversal
        va_start(args, fstring);

        // One instance of the log file
        static LOG_FILE debugLog;
        static int logfileInitialized = 0;

        // Should have mutex as well when eventually multithreaded, also in printf

        // 4K Max for a single write, maybe change in the future
        const int buflen = 4096;
        char buf[buflen];

        // Initialize if this is the first time calling
        if(!logfileInitialized) {
            logfileInitialized = 1;
            LogInit(&debugLog, "log", "DEBUG", LOG_FILEEXT_LOG);

            printf("Logging to file %s\n", debugLog.filename);

            // Optionally also fork a process to run tail -f on the log file, to print the output but not make that slow down the app
        }

        // Format string to output
        int written = vsnprintf(buf, buflen, fstring, args);

        // Write to log file
        LogUpdate(&debugLog, buf, written);

        // End variadic arguments
        va_end(args);

#endif
#ifdef DEBUG_OUT_PRINTF

        // Start variadic argument traversal
        va_start(args, fstring);

        // Format string to output
        vprintf(fstring, args);

        // End variadic arguments
        va_end(args);

#endif

    } // if (debuglevel > MASK)

} // logDebug(int, const char *, ...)


/**** Function setStdinNoncanonical ****
 *
 * Disables or re-enables input buffering and character echoing for stdin,
 * similar to curses. Must be set before it can be unset.
 *
 * Adapted from
 * https://www.gnu.org/software/libc/manual/html_node/Noncanon-Example.html
 *
 * Arguments: 
 * 	set - Nonzero to set input to noncanonical mode, zero to restore
 *
 * Return value:
 *	Does not return a value
 */
static struct termios restoreAttrs;

static void restoreStdin(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &restoreAttrs);
}

void setStdinNoncanonical(int set) {

    int rc;
    int fd = STDIN_FILENO;

    // Ensure the input is a terminal
    if (!isatty(fd)) {
        logDebug(L_INFO, "STDIN is not a terminal, refusing to set terminal mode\n");
        return;
    }

    if (set) {

        // Save old attributes
        rc = tcgetattr(fd, &restoreAttrs);
        if (rc) {
            logDebug(L_INFO, "Failed to get terminal attributes: %s\n", strerror(errno));
            return;
        }

        // Automatically restore terminal if it is not done manually
        atexit(restoreStdin);

        // Modify attributes
        struct termios newAttrs;
        rc = tcgetattr(fd, &newAttrs);
        if (rc) {
            logDebug(L_INFO, "Failed to get terminal attributes: %s\n", strerror(errno));
            return;
        }

        // Set to noncanonical mode without echo
        newAttrs.c_lflag &= ~(ICANON | ECHO);

        // Read on stdin is nonblocking
        newAttrs.c_cc[VMIN] = 0;
        newAttrs.c_cc[VTIME] = 0;

        // Take effect immediately
        rc = tcsetattr(fd, TCSANOW, &newAttrs);
        if (rc) {
            logDebug(L_INFO, "Failed to set terminal attributes: %s\n", strerror(errno));
            return;
        }

    } else {

        // Put terminal back the way you found it
        // This is done automatically at program exit
        restoreStdin();
    }

} // setStdinNoncanonical(int)


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
 * 	suf     - Numeric 4-byte suffix to put at end of new filename
 * 	ext     - Extension to put at end of new filename
 *
 * Return value:
 * 	Returns the number of characters written to buf (length of new string)
 */
int generateFilename(char *buf, int bufSize, time_t *filetime, 
        const char *dir, const char *pre, unsigned suf, const char *ext) {

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
            "%s/%s-%02d.%02d.%04d_%02d-%02d-%02d_%08x.%s",
            dir, pre,
            currentTime.tm_mon + 1,
            currentTime.tm_mday,
            currentTime.tm_year + 1900,
            currentTime.tm_hour,
            currentTime.tm_min,
            currentTime.tm_sec,
            suf,
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

