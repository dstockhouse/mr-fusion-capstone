/****************************************************************************
 *
 * File:
 * 	debuglog.c
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
 ***************************************************************************/

#include "control.h"
#include "logger.h"
#include "debuglog.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

// The logDebug function's definition depends on DEBUG_OUT_**** set in control.h
// Only one can be defined. Default to printf if none defined.

int logDebug(const char *fstring, ...) {

#ifdef DEBUG_OUT_SYSLOG

	// Initialize if this is the first time calling
	static int initialized = 0;
	if(!initialized) {

		// Initialize syslog for the program.
		// Every log message starts with "ICARUS_HFNAV"
		// PID included in log, errors logged to console
		// Logged at user level
		// Mask only DEBUG log messages
		char *syslogID = "ICARUS_HFNAV";
		openlog(syslogID, LOG_PID | LOG_CONS, LOG_USER);
		setlogmask(LOG_MASK(LOG_DEBUG));
		initialized = 1;

		printf("Logging to syslog under ID %s\n", syslogID);
	}

	// Variadic arguments to pass to syslog
	va_list args;
	va_start(args, fstring);

	// Format string to output
	vsyslog(LOG_DEBUG, fstring, args);

	// End variadic arguments
	va_end(args);

#else
#ifdef DEBUG_OUT_LOGFILE

	// One instance of the log file
	static LOG_FILE debugLog;
	static int initialized = 0;

	// Should have mutex as well when eventually multithreaded, also in printf

	// 4K Max for a single write, maybe change in the future
	const int buflen = 4096;
	char buf[buflen];

	// Initialize if this is the first time calling
	if(!initialized) {
		LogInit(&debugLog, "log", "DEBUG", LOG_FILEEXT_LOG);
		initialized = 1;

		printf("Logging to file %s\n", debugLog.filename);
	}

	// Variadic arguments to pass to sprintf
	va_list args;
	va_start(args, fstring);

	// Format string to output
	int written = vsnprintf(buf, buflen, fstring, args);

	/***********************************************************************************************
	 ***********************************************************************************************
	 ***********************************************************************************************
	 ***********************************************************************************************
	 ***********************************************************************************************
	 ***********************************************************************************************
	 ***********************************************************************************************/
	// vprintf(fstring, args);

	// Write to log file
	LogUpdate(&debugLog, buf, written);

	// End variadic arguments
	va_end(args);

#else
#ifdef DEBUG_OUT_PRINTF

	// Variadic arguments to pass to printf
	va_list args;
	va_start(args, fstring);

	// Format string to output
	vprintf(fstring, args);

	// End variadic arguments
	va_end(args);

#endif
#endif
#endif


}

