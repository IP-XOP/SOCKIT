/*
 *  Connections.h
 *  iPeek
 *
 *  Created by andrew on 17/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "XOPStandardHeaders.h"
#include "defines.h"

class waveBufferInfo {
	public:
	waveHndl bufferWave;
	bool toPrint;
	char processor[MAX_OBJ_NAME+1];
	char tokenizer[35];
};

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITcallProcessorStruct {
	Handle bufferWave;
	DOUBLE entryRow;
}SOCKITcallProcessorStruct, *SOCKITcallProcessorStructPtr;
#include "XOPStructureAlignmentReset.h"


/* A structure to hold all the socket IO information */
class CurrentConnections{
	public:
	static void Instance();
	~CurrentConnections();
	
	void resetCurrentConnections();
	SOCKET getMaxSockNumber();
	void resetMaxSocketNumber();
	int closeWorker(SOCKET sockNum);
	void addWorker(SOCKET,waveBufferInfo);
	int checkIfWaveInUseAsBuf(waveHndl wav);
	int registerProcessor(SOCKET, const char *);
	int checkProcessor(SOCKET, FunctionInfo *);
	fd_set* getReadSet();
	const waveBufferInfo* getWaveBufferInfo(SOCKET);
	int checkRecvData();
	int outputBufferDataToWave(SOCKET , const char *);
	
	private:
	CurrentConnections();
	CurrentConnections(const CurrentConnections&){};
	CurrentConnections& operator=(const CurrentConnections&);
	
	//members
	fd_set readSet;
	SOCKET maxSockNumber;
	std::map<SOCKET,waveBufferInfo> bufferWaves;	//socket descriptor, wave buffer, containing the recv messages.

	};


