#include "SOCKIT.h"
#include "SOCKITsendnrecv.h"

int
RegisterSOCKITsendnrecv(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the SOCKITopenconnectionRuntimeParams structure as well.
	cmdTemplate = "SOCKITsendnrecv/FILE=string/TIME=number/SMAL number:ID,string:MSG";
	runtimeNumVarList = "V_Flag";
	runtimeStrVarList = "S_tcp";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITsendnrecvRuntimeParams), (void*)ExecuteSOCKITsendnrecv, 0);
}

int 
ExecuteSOCKITsendnrecv(SOCKITsendnrecvRuntimeParams *p){
	int err = 0, err2=0;
	
	extern CurrentConnections *pinstance;
	
#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif
	
	MemoryStruct chunk;
	
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
	char *output = NULL;
    
	SOCKET maxSockNum = pinstance->getMaxSockNumber();
	
	fd_set tempset;
	FD_ZERO(&tempset);
	
	double timeoutVal=1.;
	struct timeval timeout;
	
	if(p->TIMEFlagEncountered){
		timeoutVal = p->TIMEFlagNumber;
	} else {
		timeoutVal = 1;
	}
	
	timeout.tv_sec = floor(timeoutVal);
	timeout.tv_usec =  (int)(timeoutVal-(double)floor(timeoutVal))*1000000;
    
	memset(buf,0,BUFLEN+1);
	memcpy(&tempset, pinstance->getReadSet(), sizeof(*(pinstance->getReadSet()))); 
	
    Handle ret = NULL;
    ret = NewHandle(0);
    if(ret == NULL){
        err = NOMEM;
        goto done;
    }
    
	if (p->MSGEncountered) {
		// Parameter: p->MSG (test for NULL handle before using)
		if(!p->MSG){
			err = OH_EXPECTED_STRING;
			goto done;
		}
		if(err = GetCStringFromHandle(p->MSG, buf, sizeof(buf)))
			goto done;
	} else {
		err = OH_EXPECTED_STRING;
		goto done;
	}
	
	if (p->IDEncountered) {
		// Parameter: p->ID
		sockNum = (SOCKET) p->ID;
	} else {
		err = OH_EXPECTED_NUMBER;
		goto done;
	}
	
	if (sockNum <= 0) {
		err = SOCKET_NOT_CONNECTED;
		goto done;
	} else {
		if(!FD_ISSET(sockNum,&tempset)){
			snprintf(report,sizeof(report),"SOCKIT err: can't write to socket %d\r", sockNum);
			XOPNotice(report);
			err2=1;
			goto done;
		}
	}
	
	// Parameter: p->FILEFlagStrH (test for NULL handle before using)
	if (p->FILEFlagEncountered) {
		// Parameter: p->FILEFlagStrH (test for NULL handle before using)
		if (p->FILEFlagStrH == NULL) {
			err = OH_EXPECTED_STRING;
			goto done;
		}
		if(err = GetCStringFromHandle(p->FILEFlagStrH,fileName,MAX_PATH_LEN))
			goto done;
		if(err = GetNativePath(fileName,fileNameToWrite))
			goto done;
		if(err = XOPOpenFile(fileNameToWrite,1,&fileToWrite))
			goto done;
	}
	
	//flush messages first
	err = pinstance->checkRecvData();
	
	//send the message second;
	res = select(maxSockNum+1,0,&tempset,0,&timeout);
	if(res == -1){
		XOPNotice ("SOCKIT err: select returned timeout");
		err2=1;
		goto done;
	}
	if(FD_ISSET(sockNum,&tempset)){
		rc = send(sockNum,buf,strlen(buf),0);
		if(rc >= 0 && pinstance->getWaveBufferInfo(sockNum)->toPrint == true){
			snprintf(report,sizeof(report),"SOCKITmsg: wrote to socket %d\r", sockNum);
			XOPNotice(report);
			output = NtoCR(buf, "\n","\r");
			XOPNotice(output);
			XOPNotice("\r");
		} else if (rc < 0) {
			snprintf(report,sizeof(report),"SOCKIT err: problem writing to socket descriptor %d, disconnecting\r", sockNum );
			XOPNotice(report);
			// Closed connection or error 
			pinstance->closeWorker(sockNum);
			err2=1;
			goto done;
		}
	} else {
		snprintf(report,sizeof(report),"SOCKIT err: timeout writing to socket %d\r", sockNum);
		XOPNotice(report);
		err2=1;
		goto done;
	}
	
	//now get an immediate reply
	memcpy(&tempset, pinstance->getReadSet(), sizeof(*(pinstance->getReadSet()))); 
	timeout.tv_sec = floor(timeoutVal);
	timeout.tv_usec =  (timeoutVal-(double)floor(timeoutVal))*1000000;
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
				pinstance->closeWorker(sockNum);
				err2=1;
				break;
			} else if(rc > 0){
				chunk.WriteMemoryCallback(buf, sizeof(char), rc);
				if(chunk.getData() == NULL){
					err = NOMEM;
					goto done;
				}
				if(fileToWrite){//write to file as well
					written = fwrite(buf, sizeof(char),rc, (FILE *)fileToWrite);
				}
			} else if (rc == 0)
				break;
			
			if(p->SMALFlagEncountered){
				res=0;
			} else {
				FD_ZERO(&tempset);
				timeout.tv_sec = floor(timeoutVal);
				timeout.tv_usec =  (timeoutVal-(double)floor(timeoutVal))*1000000;
				FD_SET(sockNum,&tempset);
				res = select(sockNum+1,&tempset,0,0,&timeout);
			}
		}while(res>0);
	} else if(res==-1) {
		snprintf(report,sizeof(report),"SOCKIT err: timeout while reading socket descriptor %d, disconnecting\r", sockNum );
		XOPNotice(report);
		// Closed connection or error 
		pinstance->closeWorker(sockNum);
		err2=1;
		goto done;
	}
	
	if (err = PutCStringInHandle(chunk.getData(),ret))
		goto done;
	
	if(err = pinstance->outputBufferDataToWave(sockNum, chunk.getData()))
		goto done;
	
done:
	if(fileToWrite){
		if(XOPCloseFile(fileToWrite))
			err = PROBLEM_WRITING_TO_FILE;
	}
	if(err || err2){
		SetOperationNumVar("V_flag", 1);
		SetOperationStrVar("S_tcp", "");		
	} else {
		SetOperationNumVar("V_flag", 0);
		SetOperationStrVar("S_tcp", chunk.getData());
	}
	
	if(output)
		free(output);
	
	FD_ZERO(&tempset);
	
	return err;
}