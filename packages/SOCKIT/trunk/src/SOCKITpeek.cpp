/*
 *  SOCKITpeek.cpp
 *  iPeek
 *
 *  Created by andrew on 25/04/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "SOCKIT.h"
#include "SOCKITpeek.h"

int SOCKITpeek(SOCKITpeekStructPtr p){
	int err = 0;
	//this function returns the entirety of the stored byte buffer for a given socket.
	//it can be called from a threadsafe function.
	
	Handle dest;
	SOCKET sockID = 0;
	extern CurrentConnections *pinstance;

	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
		
	dest = NewHandle(0);

	if(!p->sockID){
		err = OH_EXPECTED_NUMBER;
		goto done;
	}
	
	sockID = (SOCKET)doubleToLong(roundDouble(p->sockID));

	if(pinstance->isSockitOpen(p->sockID,&sockID)){
		if(pinstance->getWaveBufferInfo(sockID)->readBuffer.getData()){
			if(err = PtrAndHand((void*)pinstance->getWaveBufferInfo(sockID)->readBuffer.getData(), dest, pinstance->getWaveBufferInfo(sockID)->readBuffer.getMemSize()))
				goto done;
			pinstance->getWaveBufferInfo(sockID)->readBuffer.reset();
			if(pinstance->getWaveBufferInfo(sockID)->toClose){
				pinstance->closeWorker(sockID);
			}
		}
	} else {
		err = -1;
	}
	
done:
	
	p->dest = dest;

	if(err && dest)
		DisposeHandle(dest);

	pthread_mutex_unlock( &readThreadMutex );
	
	return err;
};
