/*
 *  SOCKITprocessor.cpp
 *  SOCKIT
 *
 *  Created by andrew on 25/04/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "CurrentConnections.h"
#include "SOCKITprocessor.h"

extern "C" int
SOCKITregisterProcessor(SOCKITprocessorStruct *p){
	int err = 0;
	char processor[MAX_OBJ_NAME+1];
	
//	extern CurrentConnections *pinstance;
//	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
	
	SOCKET sockNum;
	
    p->retval = 0;
    
	fd_set tempset;
	FD_ZERO(&tempset);
	memcpy(&tempset, pinstance->getReadSet(), sizeof(*(pinstance->getReadSet()))); 
	
	sockNum = (SOCKET)p->sockNum;
			
	if(!FD_ISSET(sockNum,&tempset)){
		err = NO_SOCKET_DESCRIPTOR;
		goto done;
	}
	
	if(err = GetCStringFromHandle(p->processor,processor,MAX_OBJ_NAME))
		goto done;
	
	if(err = pinstance->registerProcessor(sockNum, processor))
		goto done;
	
done:
	if(p->processor)
		WMDisposeHandle(p->processor);
    if(err)
        p->retval = -1;
	pthread_mutex_unlock( &readThreadMutex );

return err;
};
