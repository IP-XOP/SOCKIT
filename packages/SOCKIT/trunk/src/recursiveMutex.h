/*
 *  recursiveMutex.h
 *  iPeek
 *
 *  Created by andrew on 28/04/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

// Header file

#ifdef _MACINTOSH_
#include <pthread.h>
#endif
#include "pthread.h"


#define check_compile_time( X ) \
extern int DebugUniqueName[ 1 / ( int )( ( X ) ) ]

#define DebugUniqueName DebugMakeNameWrapper( __LINE__ )
#define DebugMakeNameWrapper( X ) DebugMakeName( X )
#define DebugMakeName( X ) check_compile_time_ ## X

typedef struct Lock Lock;

struct Lock {
uint32_t lockCount;
pthread_t lockOwner;
pthread_mutex_t lock;
};

check_compile_time( sizeof( pthread_t ) == 4 );
check_compile_time( sizeof( uint32_t ) == 4 );
check_compile_time( ( offsetof( Lock, lockCount ) & 3 ) == 0 );
check_compile_time( ( offsetof( Lock, lockOwner ) & 3 ) == 0 );

void RecursiveLock( Lock *inLock );
void RecursiveUnlock( Lock *inLock );
