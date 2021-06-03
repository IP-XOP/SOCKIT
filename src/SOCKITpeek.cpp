/*
 *  SOCKITpeek.cpp
 *  SOCKIT
 *
 *  Created by andrew on 25/04/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "CurrentConnections.h"
#include "SOCKITpeek.h"

int SOCKITpeek(SOCKITpeekStructPtr p){
	int err = 0;
	//this function returns the entirety of the stored byte buffer for a given socket.
	//it can be called from a threadsafe function.
	
	Handle dest;
	SOCKET sockID = 0;
//	extern CurrentConnections *pinstance;
//
//	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
		
	dest = WMNewHandle(0);
	if(dest == NULL){
		err = NOMEM;
		goto done;
	}

	if(!p->sockID){
		err = OH_EXPECTED_NUMBER;
		goto done;
	}
	
	sockID = (SOCKET)p->sockID;

	if(pinstance->isSockitOpen(p->sockID, &sockID)){
		if(pinstance->getWaveBufferInfo(sockID)->readBuffer.length()){
			if(err = WMPtrAndHand((void*)pinstance->getWaveBufferInfo(sockID)->readBuffer.data(), dest, pinstance->getWaveBufferInfo(sockID)->readBuffer.length())){
				dest = NULL;
				goto done;
			}
			pinstance->getWaveBufferInfo(sockID)->readBuffer.clear();
			if(pinstance->getWaveBufferInfo(sockID)->toClose){
				pinstance->closeWorker(sockID);
			}
		}
	}

done:
	pthread_mutex_unlock( &readThreadMutex );
	
	p->dest = NULL;	// Init to NULL
	if (err) {
		if (dest != NULL)
			WMDisposeHandle(dest);
		return err;
	}
	p->dest = dest;
	
	return err;
};
