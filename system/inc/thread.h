/****************************************************************************
 *
 * File:
 *      thread.h
 *
 * Description:
 *      Function and type declarations and constants for thread.c
 *
 * Author:
 *      David Stockhouse
 *
 * Revision 0.1
 *      Last edited 04/10/2020
 *
 ***************************************************************************/

#ifndef __THREAD_H
#define __THREAD_H

int ThreadAttrInit(pthread_attr_t *threadAttr, int priority);

int ThreadCreate(pthread_t *thread, pthread_attr_t *threadAttr, void *(*threadRoutine)(void *), void *threadParams);

int ThreadTryJoin(pthread_t thread, int *threadReturn);

#endif // __THREAD_H

