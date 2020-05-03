/****************************************************************************
 *
 * File:
 *      loggertest.c
 *
 * Description:
 *      CGreen test suite for the logger module (logger.c)
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 04/11/2020
 *
 ***************************************************************************/

#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>

#include "config.h"

#include "logger.h"

// Name of test context
Describe(Logger);

// Execute in the context immediately before each "Ensure" test
BeforeEach(Logger) {
}

// Execute after each test
AfterEach(Logger) {
}


/**** Function setup_sockets
 *
 * This function is the simplest example of how to properly initialize a socket
 * connection, without taking advantage of the nonblocking ability. It shows the
 * correct order to initiate API calls.
 *
 ****/
void setup_sockets(int perform_asserts) {

}


/**** Start test suite ****/

Ensure(Logger, generate_filename_and_mkdir_p) {

    char filename[LOG_FILENAME_LENGTH];
    int length, rc;

    length = generateFilename(filename, LOG_FILENAME_LENGTH, NULL, "bld/test", "LOGTEST", "d");
    assert_that(length, is_equal_to(strlen(filename)));

    rc = mkdir_p(filename, 0777);
    assert_that(rc, is_equal_to(0));

}

Ensure(Logger, successful_init) {

    char command[LOG_FILENAME_LENGTH*2];
    int length, rc;
    LOG_FILE logger;

    rc = LogInit(&logger, "bld/test", "LOGTEST", LOG_FILEEXT_LOG);
    assert_that(rc, is_equal_to(0));

    // Test if the file exists
    snprintf(command, LOG_FILENAME_LENGTH*2, "test -f %s", logger.filename);
    rc = system(command);
    assert_that(rc, is_equal_to(0));

    rc = LogClose(&logger);
    assert_that(rc, is_equal_to(0));

}

Ensure(Logger, successful_update) {

    char command[LOG_FILENAME_LENGTH*2];
    int length, rc;
    LOG_FILE logger;

    rc = LogInit(&logger, "bld/test", "LOGTEST", LOG_FILEEXT_LOG);
    assert_that(rc, is_equal_to(0));

    const char *message = "howdy";
    rc = LogUpdate(&logger, message, strlen(message));
    assert_that(rc, is_equal_to(strlen(message)));

    // Test if the file has the expected contents
    LogFlush(&logger);
    snprintf(command, LOG_FILENAME_LENGTH*2, "echo -n '%s' | cmp %s", message, logger.filename);
    rc = system(command);
    assert_that(rc, is_equal_to(0));

    rc = LogClose(&logger);
    assert_that(rc, is_equal_to(0));

}

