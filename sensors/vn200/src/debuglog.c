/****************************************************************************
 *
 * File:
 * 	debuglog.c
 *
 * Description:
 * 	Logs data for the program run in a consistent location
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 10/11/2019
 *
 * Revision 0.2
 *  Fixed a bug in how variadic arguments are traversed
 * 	Last edited 2/13/2020
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "control.h"
#include "logger.h"

#include "debuglog.h"

// The logDebug function's definition depends on DEBUG_OUT_**** set in control.h
// 1-3 options can be defined individually or simultaneously, in which case
// debug will be logged in more than one location

int logDebug(int debuglevel, const char *fstring, ...) {

    // Only log anything if the level is higher than the allowed mask
    if (debuglevel > CONTROL_DEBUG_L_MASK) {

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

} // logDebug(const char *, ...)

