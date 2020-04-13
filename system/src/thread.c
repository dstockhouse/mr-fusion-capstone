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

#define _GNU_SOURCE

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
 *      threadAttr      - Pointer to thread attributes object to set
 *      inversePriority - Inverted priority to set thread scheduler (0 is highest)
 *
 * Return value:
 *      On success, returns 0
 *      On failure, returns -1 and errno is set to the value returned by the
 *        failed system service call
 */
int ThreadAttrInit(pthread_attr_t *threadAttr, int inversePriority) {

    int rc, priority, maxPriority, minPriority, tempPriority;
    struct sched_param schedParams;

    // Null not automatically rejected, so reject it here
    if (threadAttr == NULL) {
        // Invalid input
        errno = EINVAL;
        return -1;
    }

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

    // Get maximum and minimum supported priority
    maxPriority = sched_get_priority_max(SCHED_FIFO);
    if (maxPriority == -1) {
        logDebug(L_INFO, "%s: Failed to get maximum sched priority\n", strerror(errno));
        return -1;
    }
    minPriority = sched_get_priority_min(SCHED_FIFO);
    if (minPriority == -1) {
        logDebug(L_INFO, "%s: Failed to get minimum sched priority\n", strerror(errno));
        return -1;
    }

    // Invert input priority, check bounds
    tempPriority = maxPriority - inversePriority;
    if (tempPriority > maxPriority) {
        priority = maxPriority;
    } else if (tempPriority < minPriority) {
        priority = minPriority;
    } else {
        priority = tempPriority;
    }
    logDebug(L_DEBUG, "Setting thread priority to %d\n", priority);
    logDebug(L_VVDEBUG, "  (min = %d, max = %d)\n", minPriority, maxPriority);

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
int ThreadTryJoin(pthread_t thread, int *threadReturn) {

    int rc, *tempReturn = NULL;

    // Attempt to join thread
    // Not supported on non-Linux systems (ex. Cygwin)
    rc = pthread_tryjoin_np(thread, (void **)&tempReturn);
    // rc = pthread_join(thread, (void **)&tempReturn);
    if (rc != 0) {
        if (rc != EBUSY) {
            logDebug(L_INFO, "%s: Failed to join thread\n", strerror(rc));
        }
        errno = rc;
        return -1;
    }

    // Save return value, if present and required
    if (threadReturn != NULL && tempReturn != NULL) {
        *threadReturn = *tempReturn;
    }

    return 0;

} // ThreadTryJoin()

