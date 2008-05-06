#include "SOCKIT.h"


int 
SOCKITsendnrecv(SOCKITsendnrecvStruct *p){
	int err = 0;
	p->retval = 0;
	
	extern currentConnections openConnections;
	
#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif
	
	struct MemoryStruct chunk;
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
    chunk.size = 0;    /* no data at this point */
	
	XOP_FILE_REF fileToWrite = NULL;
	char fileName[MAX_PATH_LEN+1];
	char fileNameToWrite[MAX_PATH_LEN+1];
	int written;
	
    int rc = 0;
	long charsread = 0;
	
    SOCKET sockNum = -1;
	int res = 0;
	
	char buf[BUFLEN+1];
	char report[MAX_MSG_LEN+1];
	char* errVar = "V_Flag";
    
	SOCKET maxSockNum = openConnections.maxSockNumber;
	
	fd_set tempset;
	FD_ZERO(&tempset);
	
	struct timeval timeout;
	timeout.tv_sec = floor(p->timeout);
	timeout.tv_usec =  (int)(p->timeout-(double)floor(p->timeout))*1000000;
    
	memset(buf,0,BUFLEN+1);
	memcpy(&tempset, &openConnections.readSet, sizeof(openConnections.readSet)); 
	
    Handle ret = NULL;
    ret = NewHandle(0);
    if(ret == NULL){
        err = NOMEM;
        goto done;
    }
    
	if(!p->message){
		err = OH_EXPECTED_STRING;
		goto done;
	} else {
		if(err = GetCStringFromHandle(p->message, buf, sizeof(buf)))
			goto done;
	}
	
	if(!p->sockNum){
		err = OH_EXPECTED_NUMBER;
		goto done;
	}
	
	sockNum = (SOCKET) p->sockNum;
	
	if (sockNum <= 0) {
		err = SOCKET_NOT_CONNECTED;
		goto done;
	} else {
		if(!FD_ISSET(sockNum,&tempset)){
			snprintf(report,sizeof(report),"SOCKIT err: can't write to socket %d\r", sockNum);
			XOPNotice(report);
			goto done;
		}
	}
	
	// Parameter: p->FILEFlagStrH (test for NULL handle before using)
	if (p->fileName == NULL) {
		err = OH_EXPECTED_STRING;
		goto done;
	}
	
	if(err = GetCStringFromHandle(p->fileName,fileName,MAX_PATH_LEN))
		goto done;
	if(strlen(fileName) > 0){
		if(err = GetNativePath(fileName,fileNameToWrite))
			goto done;
		if(err = XOPOpenFile(fileNameToWrite,1,&fileToWrite))
			goto done;
	}
	
	//flush messages first
	err = checkRecvData();
	
	//send the message second;
	res = select(maxSockNum+1,0,&tempset,0,&timeout);
	if(res == -1){
		XOPNotice ("SOCKIT err: select returned timeout");
		SetIgorIntVar(errVar, 1, 1);
		goto done;
	}
	if(FD_ISSET(sockNum,&tempset)){
		rc = send(sockNum,buf,strlen(buf),0);
		if(rc >= 0 && openConnections.bufferWaves[sockNum].toPrint == true){
			snprintf(report,sizeof(report),"SOCKITmsg: wrote to socket %d\r", sockNum);
			XOPNotice(report);
			snprintf(report,sizeof(report),"%s\r",buf);
			XOPNotice(buf);
		} else if (rc < 0) {
			snprintf(report,sizeof(report),"SOCKIT err: problem writing to socket descriptor %d, disconnecting\r", sockNum );
			XOPNotice(report);
			// Closed connection or error 
			SOCKITcloseWorker(sockNum);
			SetIgorIntVar(errVar, 1, 1);
			goto done;
		}
	} else {
		snprintf(report,sizeof(report),"SOCKIT err: timeout writing to socket %d\r", sockNum);
		XOPNotice(report);
		SetIgorIntVar(errVar, 1, 1);
		goto done;
	}
	
	//now get an immediate reply
	memcpy(&tempset, &openConnections.readSet, sizeof(openConnections.readSet)); 
	timeout.tv_sec = floor(p->timeout);
	timeout.tv_usec =  (p->timeout-(double)floor(p->timeout))*1000000;
	res = select(maxSockNum+1,&tempset,0,0,&timeout);
	
	memset(buf,0,BUFLEN);
	
	if (res && FD_ISSET(sockNum, &tempset)) {            
		do{			   
#ifdef _MACINTOSH_
			rc = recv(sockNum, buf, BUFLEN,0);
#endif
#ifdef _WINDOWS_
			rc = recv(sockNum, buf, BUFLEN,0);
#endif
			charsread += rc;
			
			if (rc < 0) { 
				snprintf(report,sizeof(report),"SOCKIT err: problem reading socket descriptor %d, disconnection???\r", sockNum );
				XOPNotice(report);
				// Closed connection or error 
				SOCKITcloseWorker(sockNum);
				SetIgorIntVar(errVar, 1, 1);
				break;
			} else if(rc > 0){
				WriteMemoryCallback(buf, sizeof(char), rc, &chunk);
				if(chunk.memory == NULL){
					err = NOMEM;
					goto done;
				}
				if(fileToWrite){//write to file as well
					written = fwrite(buf, sizeof(char),rc, (FILE *)fileToWrite);
				}
			} else if (rc == 0)
				break;
				
			FD_ZERO(&tempset);
			timeout.tv_sec = floor(p->timeout);
			timeout.tv_usec =  (p->timeout-(double)floor(p->timeout))*1000000;
			FD_SET(sockNum,&tempset);
			res = select(sockNum+1,&tempset,0,0,&timeout);
			
		}while(res>0);
	} else if(res==-1) {
		snprintf(report,sizeof(report),"SOCKIT err: timeout while reading socket descriptor %d, disconnecting\r", sockNum );
		XOPNotice(report);
		// Closed connection or error 
		SOCKITcloseWorker(sockNum);
		SetIgorIntVar(errVar, 1, 1);
		goto done;
	}
	
	WriteMemoryCallback((char*)"\0", sizeof(char), strlen((char*)"\0"), &chunk);
	if(chunk.memory == NULL){
		err = NOMEM;
		goto done;
	}
	
	if (err = PutCStringInHandle(chunk.memory,ret))
		goto done;
	
	if(err = outputBufferDataToWave(sockNum, openConnections.bufferWaves[sockNum].bufferWave, chunk.memory, openConnections.bufferWaves[sockNum].tokenizer))
		goto done;
	
done:
	if(fileToWrite){
		if(XOPCloseFile(fileToWrite))
			err = PROBLEM_WRITING_TO_FILE;
	}
	if(chunk.memory)
		free(chunk.memory);
	if (p->message)
		DisposeHandle(p->message);			/* we need to get rid of input parameters */
	if (p->fileName)
		DisposeHandle(p->fileName);
	if(err){
		SetIgorIntVar(errVar, 1, 1);
	} else SetIgorIntVar(errVar,0,1);
	
	//populate the string
	p->retval = ret;
	
	FD_ZERO(&tempset);
	
	
	return err;
}