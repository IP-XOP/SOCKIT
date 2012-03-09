/*
 *  SOCKITlist.cpp
 *  SOCKIT
 *
 *  Created by andrew on 13/06/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#include "CurrentConnections.h"
#include "SOCKITlist.h"

int ExecuteSOCKITlist(SOCKITlistRuntimeParamsPtr p){
	int err = 0;

	extern CurrentConnections* pinstance;
	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
	vector<SOCKET> openSockets;
	waveHndl waveH;
	char waveName[MAX_OBJ_NAME+1];
	CountInt dimensionSizes[MAX_DIMENSIONS+1];
	long *data;
	
	strncpy(waveName, "W_Sockitlist", MAX_OBJ_NAME);
		
	memset(dimensionSizes, 0, (MAX_DIMENSIONS + 1) * sizeof(long));
	dimensionSizes[0] = pinstance->getCurrentSocketsOpened();
	
	if(err = MDMakeWave(&waveH, waveName, NULL, dimensionSizes, NT_I32 | NT_UNSIGNED, 1))
		goto done;
	
	pinstance->getListOfOpenSockets(openSockets);
	
	if(openSockets.size()){
		data = (long*) WaveData(waveH);
		memcpy(data, &(openSockets[0]), openSockets.size() * sizeof(long));
	}
	
done:
	
	pthread_mutex_unlock( &readThreadMutex );
	return err;
	
}


int
RegisterSOCKITlist(void)
{
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the SOCKITlistRuntimeParams structure as well.
	cmdTemplate = "SOCKITlist";
	runtimeNumVarList = "";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITlistRuntimeParams), (void*)ExecuteSOCKITlist, 0);
}