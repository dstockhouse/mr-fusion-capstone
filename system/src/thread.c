/****************************************************************************
 *
 * File:
 *      thread.c
 *
 * Description:
 *      Abstraction of pthread library for thread management and creation
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 04/10/2020
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>

#include "debuglog.h"

#include "thread.h"


/**** Function ThreadAttrInit ****
 *
 * Initializes thread attributes for realtime scheduling and priority
 *
 * Arguments: 
 *      threadAttr - Pointer to thread attributes object to set
 *      priority   - Integer priority to set thread scheduler
 *
 * Return value:
 *      On success, returns 0
 *      On failure, returns -1 and errno is set to the value returned by the
 *        failed system service call
 */
int ThreadAttrInit(pthread_attr_t *threadAttr, int priority) {

    int rc;
    struct sched_param schedParams;

    // Initialize attributes object
    rc = pthread_attr_init(threadAttr);
    if (rc != 0) {
        logDebug(L_INFO, "%s: Failed to initialize thread attributes\n", strerror(rc));
        errno = rc;
        return -1;
    }

    // Set to not inherit scheduling attributes from spawning thread
    rc = pthread_attr_setinheritsched(threadAttr, PTHREAD_EXPLICIT_SCHED);
    if (rc != 0) {
        logDebug(L_INFO, "%s: Failed to set inherit sched\n", strerror(rc));
        errno = rc;
        return -1;
    }

    // Set realtime scheduler
    rc = pthread_attr_setschedpolicy(threadAttr, SCHED_FIFO);
    if (rc != 0) {
        logDebug(L_INFO, "%s: Failed to set sched policy\n", strerror(rc));
        errno = rc;
        return -1;
    }

    // Set scheduling priority
    schedParams.sched_priority = priority;
    rc = pthread_attr_setschedparam(threadAttr, &schedParams);
    if (rc != 0) {
        logDebug(L_INFO, "%s: Failed to set sched priority\n", strerror(rc));
        errno = rc;
        return -1;
    }

    // Other parameters being left at defaults:
    // For detach state other than default, see pthread_attr_setdetachstate
    // For CPU affinity (AMP), see pthread_attr_setaffinity_np and CPU_SET
    // For CPU contention scope, see pthread_attr_setscope
    // For stack attributes, see pthread_attr_setstack

    // Return success
    return 0;

} // ThreadAttrInit()


/**** Function ThreadCreate ****
 *
 * Initializes thread attributes for realtime scheduling and priority
 *
 * Arguments: 
 *      thread        - Pointer to thread object to be created
 *      threadAttr    - Pointer to thread attributes object to set
 *      threadRoutine - Pointer to function for thread to execute
 *      threadParams  - Pointer to parameters to pass to threadRoutine
 *
 * Return value:
 *      On success, returns 0
 *      On failure, returns -1 and errno is set to the value returned by the
 *        failed system service call
 */
int ThreadCreate(pthread_t *thread, pthread_attr_t *threadAttr, void *(*threadRoutine)(void *), void *threadParams) {

    int rc;

    // Create thread using all input parameters
    rc = pthread_create(thread, threadAttr, threadRoutine, threadParams);
    if (rc != 0) {
        logDebug(L_INFO, "%s: Failed to create thread\n", strerror(rc));
        errno = rc;
        return -1;
    }

    return 0;

} // ThreadCreate()


/**** Function ThreadTryJoin ****
 *
 * Checks if a given thread is finished, and if so returns the return value
 *
 * Arguments: 
 *      thread       - Pointer to thread object to be joined
 *      threadReturn - Pointer to integer to hold an integer return value
 *
 * Return value:
 *      On success (thread was joined), returns 0
 *      On failure (thread was not joined), returns -1 and errno is set to the
 *        value returned by the failed system service call
 */
int ThreadTryJoin(pthread_t *thread, int *threadReturn) {

    int rc;

    // Attempt to join thread
    // rc = pthread_tryjoin_np(thread, &threadReturn);
    rc = pthread_join(thread, &threadReturn);
    if (rc != 0) {
        if (rc != EBUSY) {
            logDebug(L_INFO, "%s: Failed to join thread\n", strerror(rc));
        }
        errno = rc;
        return -1;
    }

    return 0;

} // ThreadTryJoin()

