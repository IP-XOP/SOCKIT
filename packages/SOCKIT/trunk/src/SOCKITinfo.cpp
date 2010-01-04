/*
 *  SOCKITinfo.cpp
 *  SOCKIT
 *
 *  Created by andrew on 4/01/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "SOCKIT.h"
#include "SOCKITinfo.h"

//return how many sockets have been opened historically
int SOCKITtotalOpened(SOCKITtotalOpenedStruct *p){
	int	err = 0;
	extern CurrentConnections *pinstance;
	
	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
	
	p->retval = pinstance->getTotalSocketsOpened();
	
done:
	
	pthread_mutex_unlock( &readThreadMutex );
	return err;
};

//return how many sockets are opened at the moment
int SOCKITcurrentOpened(SOCKITtotalOpenedStruct *p){
	int	err = 0;
	extern CurrentConnections *pinstance;
	
	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
	
	p->retval = pinstance->getCurrentSocketsOpened();
	
done:
	
	pthread_mutex_unlock( &readThreadMutex );
	return err;
};
