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

xEnsure(Logger, dummy) {

    assert_that(0, is_equal_to(1));

}

