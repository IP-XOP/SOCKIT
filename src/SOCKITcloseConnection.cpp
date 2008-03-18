#include "SOCKIT.h"



void
resetMaxSocketNumber(){
	extern currentConnections openConnections;
	int ii=0;
	int maxSoFar=0;

	for (ii=0; ii< openConnections.maxSockNumber+1 ; ii+=1){
		if (FD_ISSET(ii, &(openConnections.readSet)))
			maxSoFar = ii;
	}
	openConnections.maxSockNumber = maxSoFar;
}

int checkIfWaveInUseAsBuf(waveHndl wav){
	int inUse = 0;
	extern currentConnections openConnections;
	int ii;
	
	waveBufferInfoStruct bufferStruct;
	
	for (ii=0; ii< openConnections.maxSockNumber+1 ; ii+=1){
		if (FD_ISSET(ii, &(openConnections.readSet))) { 
			bufferStruct = openConnections.bufferWaves[ii];
			if(bufferStruct.bufferWave == wav)
				return 1;
		} 
	}
	
	return inUse;
}

int SOCKITcloseWorker(SOCKET socketToClose){
	int err = 0;

	extern currentConnections openConnections;
	char  report[MAX_MSG_LEN+1];
    
	/* Disconnect from server */
	if (FD_ISSET(socketToClose, &(openConnections.readSet))) { 
		FD_CLR(socketToClose, &(openConnections.readSet)); 
		close(socketToClose);
		resetMaxSocketNumber();
		//shut down the buffering
		openConnections.bufferWaves.erase(socketToClose);
		snprintf(report,sizeof(report),"SOCKITmsg:  Closed connection to socket descriptor %d\r", socketToClose);
		XOPNotice(report);
		err = 0;
	} else {
		snprintf(report,sizeof(report),"SOCKIT err: there is no open socket with descriptor number %d\r", socketToClose);
		XOPNotice(report);
		err = -1;
	}

done:

return err;
}

int SOCKITcloseConnection(SOCKITcloseConnectionStruct *p){
	int err = 0;
	
	SOCKET socketToClose = 0;
	
	extern currentConnections openConnections;
	SOCKET ii;
 	
	if(!p->socketToClose){
		err = OH_EXPECTED_NUMBER;
		p->retval = -1;
		goto done;
	} else if (p->socketToClose = -1){
		for (ii=0; ii< openConnections.maxSockNumber+1 ; ii+=1){
			if (FD_ISSET(ii, &(openConnections.readSet))) { 
				SOCKITcloseWorker(ii);
			} 
		}
		p->retval = 0;
	} else {
		socketToClose = p->socketToClose;
		p->retval = SOCKITcloseWorker(socketToClose);
	}
	
	
done:
		
		return err;
}

