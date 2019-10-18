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
 ***************************************************************************/

#include "control.h"
#include "logger.h"
#include "debuglog.h"

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

// The logDebug function's definition depends on DEBUG_OUT_**** set in control.h
// 1-3 options can be defined, in which case debug will be logged more than once

int logDebug(const char *fstring, ...) {

	// Variadic arguments to pass to printf
	va_list args;
	va_start(args, fstring);

#ifdef DEBUG_OUT_SYSLOG

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

#endif
#ifdef DEBUG_OUT_LOGFILE

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
		printf("Initializing log to print ");
		vprintf(fstring, args);
		LogInit(&debugLog, "log", "DEBUG", LOG_FILEEXT_LOG);

		printf("Logging to file %s\n", debugLog.filename);
	}

	// Format string to output
	int written = vsnprintf(buf, buflen, fstring, args);

	// Write to log file
	LogUpdate(&debugLog, buf, written);

#endif
#ifdef DEBUG_OUT_PRINTF

	// Format string to output
	vprintf(fstring, args);

#endif

	// End variadic arguments
	va_end(args);

}

