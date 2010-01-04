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

int SOCKITtotalOpened(SOCKITtotalOpenedStruct *p){
	int	err = 0;
	//returns the truth abouot whether the given socket is still open.
	extern CurrentConnections *pinstance;
	
	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
	
	p->retval = pinstance->getTotalSocketsOpened();
	
done:
	
	pthread_mutex_unlock( &readThreadMutex );
	return err;
};
