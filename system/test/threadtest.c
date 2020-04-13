/****************************************************************************
 *
 * File:
 *      threadtest.c
 *
 * Description:
 *      CGreen test suite for the pthread abstraction module (thread.c)
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 03/19/2020
 *
 ***************************************************************************/

#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "config.h"

#include "thread.h"

int threadGlobal = 0;

// Name of test context
Describe(Thread);

// Execute in the context immediately before each "Ensure" test
BeforeEach(Thread) {
}

// Execute after each test
AfterEach(Thread) {
}


/**** Function setup_sockets
 *
 * This function is the simplest example of how to properly initialize a socket
 * connection, without taking advantage of the nonblocking ability. It shows the
 * correct order to initiate API calls.
 *
 ****/
void *setThreadGlobalToOne(void *threadParam) {

    // Not mutex protected, the simplest example possible
    threadGlobal = 1;

    // Give some return value
    return threadParam;

}


/**** Start test suite ****/

Ensure(Thread, attributes_init_successfully) {

    pthread_attr_t threadAttr;
    struct sched_param schedParams;
    int rc, var, priority = 10;

    // Initialize attributes with a priority of 10
    rc = ThreadAttrInit(&threadAttr, priority);
    assert_that(rc, is_equal_to(0));

    // Check important attributes of the initialized object using pthread library
    // (pthread assumed to be well tested and functional)
    rc = pthread_attr_getdetachstate(&threadAttr, &var);
    assert_that(rc, is_equal_to(0));
    assert_that(var, is_equal_to(PTHREAD_CREATE_JOINABLE));

    rc = pthread_attr_getinheritsched(&threadAttr, &var);
    assert_that(rc, is_equal_to(0));
    assert_that(var, is_equal_to(PTHREAD_EXPLICIT_SCHED));

    rc = pthread_attr_getschedpolicy(&threadAttr, &var);
    assert_that(rc, is_equal_to(0));
    assert_that(var, is_equal_to(SCHED_FIFO));

    rc = pthread_attr_getschedparam(&threadAttr, &schedParams);
    assert_that(rc, is_equal_to(0));
    assert_that(schedParams.sched_priority, is_equal_to(sched_get_priority_max(SCHED_FIFO) - priority));

}

Ensure(Thread, attributes_fail_with_null) {

    int rc;

    // Initialize attributes with a NULL pointer
    rc = ThreadAttrInit(NULL, 0);
    assert_that(rc, is_not_equal_to(0));

}

Ensure(Thread, priority_bounds_correct) {

    pthread_attr_t threadAttr;
    struct sched_param schedParams;
    int rc, priority;

    // Initialize attributes with a priority of -1
    priority = -1;
    rc = ThreadAttrInit(&threadAttr, priority);
    assert_that(rc, is_equal_to(0));

    rc = pthread_attr_getschedparam(&threadAttr, &schedParams);
    assert_that(rc, is_equal_to(0));
    assert_that(schedParams.sched_priority, is_equal_to(sched_get_priority_max(SCHED_FIFO)));

    // Initialize attributes with a priority of 100000
    priority = 100000;
    rc = ThreadAttrInit(&threadAttr, priority);
    assert_that(rc, is_equal_to(0));

    rc = pthread_attr_getschedparam(&threadAttr, &schedParams);
    assert_that(rc, is_equal_to(0));
    assert_that(schedParams.sched_priority, is_equal_to(sched_get_priority_min(SCHED_FIFO)));

}

Ensure(Thread, create_and_join) {

    int rc, threadReturn = 1000, threadParam = 20;
    pthread_attr_t threadAttr;
    pthread_t thread;

    // Initialize global
    threadGlobal = 0;

    // Initialize attributes with a priority of 0
    rc = ThreadAttrInit(&threadAttr, 0);
    assert_that(rc, is_equal_to(0));

    // Start thread to run on function with parameter
    rc = ThreadCreate(&thread, &threadAttr, &setThreadGlobalToOne, &threadParam);
    assert_that(rc, is_equal_to(0));

    // Wait for thread to finish
    int i = 0;
    do {
        usleep(100000);
        rc = ThreadTryJoin(thread, &threadReturn);
        i++;
    } while (i < 10 && rc != 0);
    assert_that(rc, is_equal_to(0));

    // Ensure the thread did its work
    assert_that(threadGlobal, is_equal_to(1));

    // Check return value
    assert_that(threadReturn, is_equal_to(threadParam));

}

Ensure(Thread, create_with_invalid_attributes) {

    int rc;
    pthread_attr_t threadAttr;
    pthread_t thread;

    // Start thread without valid attributes
    rc = ThreadCreate(&thread, &threadAttr, &setThreadGlobalToOne, NULL);
    assert_that(rc, is_equal_to(-1));

}

xEnsure(Thread, join_without_create) {

    int rc;

    // Start thread without valid attributes
    rc = ThreadTryJoin(NULL, NULL);
    assert_that(rc, is_equal_to(-1));

}

