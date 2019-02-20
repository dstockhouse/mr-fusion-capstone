/****************************************************************************\
 *
 * File:
 * 	logger.c
 *
 * Description:
 * 	Generates a filename from a timestamp and string arguments
 *
 * Author:
 * 	David Stockhouse
 *
 * Revision 0.1
 * 	Last edited 2/20/2019
 *
\***************************************************************************/

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
int generateFilename(char *buf, int bufSize, time_t *time, 
		const char *dir, const char *pre, const char *ext) {

	// Length of filename generated
	int charsWritten;

	// Time variables
	struct tm currentTime;

	// Get current time in UTC
	localtime_r(time, &currentTime);

	// Create filename using date/time and input string
	charsWritten = snprintf(buf, bufSize, 
			"%s%s-%02d.%02d.%04d_%02d-%02d-%02d.%s",
			dir, pre,
			currentTime.tm_mon + 1,
			currentTime.tm_mday,
			currentTime.tm_year + 1900,
			currentTime.tm_hour,
			currentTime.tm_min,
			currentTime.tm_sec,
			ext);

	// Return length of the new string
	return charsWritten;

} // generateFilename(char *, int, time_t, char *, char *, char *)


/**** Function initLog ****
 *
 * Creates a timestamp and log file
 *
 * Arguments:
 * 	dir - String name of the directory to create the log file in
 * 	pre - String prefix to use for the log file
 * 	bin - Boolean, 1 if binary log file, 0 if plaintext log file
 *
 * Return value:
 * 	On success, returns a file descriptor for the log file
 */
int initLog(const char *dir, const char *pre, int bin) {

	int rc, log_fd;

	char logFilename[LOG_FILENAME_LENGTH];
	int logFilenameLength;
	char ext[8];

	time_t time_var;

	// Get seconds since epoch
	time_var = time(NULL);

	// Find extension (bin or log)
	if(bin) {
		strcpy(ext, "bin");
	} else {
		strcpy(ext, "log");
	}

	// Generate filename for the log file
	logFilenameLength = generateFilename(logFilename, LOG_FILENAME_LENGTH, 
			&time_var, dir, pre, ext);
	if(logFilenameLength < LOG_FILENAME_LENGTH) {
		printf("Filename too long, using %s\n", logFilename);
	}

	// Create directory if it doesn't exist
	rc = mkdir(dir, 0777);
	if(rc && errno != EEXIST) {
		perror("Failed to create directory");
		printf(dir);
		return 1;
	}

	// Open and create the log file
	log_fd = open(logFilename, O_WRONLY | O_CREAT | O_EXCL, 0666);
	if(rc) {
		perror("Failed to create log file");
		printf(logFilename);
		return 2;
	}

	// Return the open file descriptor
	return log_fd;

} // initLog(char *, char *, int)

