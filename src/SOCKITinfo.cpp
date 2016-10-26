/*
 *  SOCKITinfo.cpp
 *  SOCKIT
 *
 *  Created by andrew on 4/01/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "CurrentConnections.h"
#include "SOCKITinfo.h"

//return how many sockets have been opened historically
int SOCKITtotalOpened(SOCKITtotalOpenedStruct *p){
	int	err = 0;
//	extern CurrentConnections *pinstance;
//	extern pthread_mutex_t readThreadMutex;
		
	pthread_mutex_lock( &readThreadMutex );
	
	p->retval = pinstance->getTotalSocketsOpened();
	
done:
	
	pthread_mutex_unlock( &readThreadMutex );
	return err;
};

//return how many sockets are opened at the moment
int SOCKITcurrentOpened(SOCKITtotalOpenedStruct *p){
	int	err = 0;
    
//	extern CurrentConnections *pinstance;
//	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
	
	p->retval = pinstance->getCurrentSocketsOpened();
	
done:
	
	pthread_mutex_unlock( &readThreadMutex );
	return err;
};

//return info about a current sockit
int SOCKITinfo(SOCKITinfoStruct *p){
	int	err = 0;
	string chunk;
	Handle result = NULL;
	char report[MAXCMDLEN + MAX_OBJ_NAME + 1];
	unsigned int szReport = (MAXCMDLEN + MAX_OBJ_NAME + 1) * sizeof(char);
	char datafoldername[MAXCMDLEN + 1];
	char bufwavename[MAX_OBJ_NAME + 1];
	DataFolderHandle dfh;
	
//	extern CurrentConnections *pinstance;
//	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
	SOCKET socknum;
	waveBufferInfo *wbi;
	
	p->retval = NULL;

	result = NewHandle(0);
	if(!result){
		err = NOMEM;
		goto done;
	}
	
	if(igorVersion < 620 && !RunningInMainThread()){
		err = NOT_IN_THREADSAFE;
		goto done;
	}
	
	//see if the socket is open, if it's not, just return a zero length string
	if(!pinstance->isSockitOpen(p->sockID, &socknum))
		goto done;

	wbi = pinstance->getWaveBufferInfo(socknum);
	
	//put in the socket number.
	snprintf(report, szReport, "ID-%d;", socknum);
	chunk.append(report, strlen(report));
	
	//put in the IP address.
	snprintf(report, szReport, "IP-%s;", wbi->hostIP);
	chunk.append(report, strlen(report));

	//put in the port.
	snprintf(report, szReport, "PORT-%s;", wbi->port);
	chunk.append(report, strlen(report));
	
	//put in the memory size
	snprintf(report, szReport, "MEMSIZE-%d;", wbi->readBuffer.length());
	chunk.append(report, strlen(report));
	
	//put in the processor name(if any).
	snprintf(report, szReport, "PROCESSOR-%s;", wbi->processor);
	chunk.append(report, strlen(report));
	
	//append the bufferwave info
	if(wbi->bufferWaveRef){
		if(err = GetWavesDataFolder(wbi->bufferWaveRef, &dfh))
			goto done;
		
		if(err = GetDataFolderNameOrPath(dfh, 1, datafoldername))
			goto done;
		
		snprintf(report, szReport, "BUFFERWAVE-%s", datafoldername);
		chunk.append(report, strlen(report));
		
		WaveName(wbi->bufferWaveRef, bufwavename);
		snprintf(report, szReport, "%s;", bufwavename);
		chunk.append(report, strlen(report));
	}
		
	if(err = PutCStringInHandle(chunk.c_str(), result))
		goto done;
		
done:
	if(result)
		p->retval = result;
	
	pthread_mutex_unlock( &readThreadMutex );
	return err;
};
