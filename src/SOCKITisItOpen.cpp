/*
 *  SOCKITisItOpen.cpp
 *  iPeek
 *
 *  Created by andrew on 28/08/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#include "CurrentConnections.h"
#include "SOCKITisItOpen.h"

extern "C" int
SOCKITisItOpen(SOCKITisItOpenStruct *p){
	int	err = 0;
	//returns the truth abouot whether the given socket is still open.
	pthread_mutex_lock( &readThreadMutex );

	SOCKET sockNum;
	int retVal;
	
	retVal = pinstance->isSockitOpen(p->sockitQuery, &sockNum);
	p->retval = retVal;
	
	pthread_mutex_unlock( &readThreadMutex );
	return err;
};

