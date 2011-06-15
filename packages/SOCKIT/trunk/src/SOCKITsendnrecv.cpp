#include "CurrentConnections.h"
#include "SOCKITsendnrecv.h"

int
RegisterSOCKITsendnrecv(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the SOCKITopenconnectionRuntimeParams structure as well.
	cmdTemplate = "SOCKITsendnrecv/FILE=string/TIME=number/NBYT=number/SMAL number:ID,string:MSG [,varname:ret]";
	runtimeNumVarList = "V_Flag";
	runtimeStrVarList = "S_tcp";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITsendnrecvRuntimeParams), (void*)ExecuteSOCKITsendnrecv, 0);
}

int 
ExecuteSOCKITsendnrecv(SOCKITsendnrecvRuntimeParams *p){
	int err = 0, err2=0;
	
	extern CurrentConnections *pinstance;
	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );


#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif
	
	string chunk;	
	
	XOP_FILE_REF fileToWrite = NULL;
	char fileName[MAX_PATH_LEN+1];
	char fileNameToWrite[MAX_PATH_LEN+1];
	int written;
	
    int rc = 0;
	long charsread = 0;
	
    SOCKET sockNum = -1;
	int res = 0;
	waveBufferInfo *wbi = NULL;
	char buf[BUFLEN+1];
	char report[MAX_MSG_LEN+1];
	string output;
	long size = 0;
	bool needToClose = false;
	long NBYTES_to_recv = -1;
	
	char timestamp[101];
		
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
	timeout.tv_usec =  (long)((timeoutVal-(double)floor(timeoutVal))*1000000);
    
	if(p->NBYTFlagEncountered){
		if(IsNaN64(&p->NBYTFlagNumber) || IsINF64(&p->NBYTFlagNumber) || p->NBYTFlagNumber < 0)
			NBYTES_to_recv = -1;

		NBYTES_to_recv = doubleToLong(roundDouble(p->NBYTFlagNumber));
	}
	memset(buf,0,sizeof(buf));
	
	if (p->MSGEncountered) {
		// Parameter: p->MSG (test for NULL handle before using)
		if(!p->MSG){
			err = OH_EXPECTED_STRING;
			goto done;
		}
		size = GetHandleSize(p->MSG);
		if(size>BUFLEN){
			err2 = 1;
			XOPNotice("SOCKIT err: message is longer than buffer\r");
			goto done;
		}
		if(err = GetCStringFromHandle(p->MSG, buf, sizeof(buf)))
			goto done;
	} else {
		err = OH_EXPECTED_STRING;
		goto done;
	}
	
	if (!p->IDEncountered) {
		err = OH_EXPECTED_NUMBER;
		goto done;
	}
	
	if (p->retEncountered) {
		if (p->retParamsSet[0]) {
			// Optional parameter: p->ret
			int dataTypePtr;
			if(err = VarNameToDataType(p->ret, &dataTypePtr))
				goto done;
			if(dataTypePtr){
				err = OH_EXPECTED_VARNAME;
				goto done;
			}
		}
	}
	
	if(!pinstance->isSockitOpen(p->ID,&sockNum)){
		err2 = SOCKET_NOT_CONNECTED;
		XOPNotice("SOCKIT err: socket not connected\r");
		goto done;
	}
	
	wbi = pinstance->getWaveBufferInfo(sockNum);
	
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
	
	//send the message second;	
	FD_SET(sockNum,&tempset);
	res = select(sockNum+1, 0, &tempset, 0, &timeout);				
	if(res == -1){
		XOPNotice ("SOCKIT err: select returned timeout\r");
		err2=1;
		goto done;
	}
	if(FD_ISSET(sockNum,&tempset)){
		rc = send(sockNum, buf, GetHandleSize(p->MSG), 0);
		if(rc > 0){
			snprintf(report,sizeof(report),"SOCKITmsg: wrote to socket %d\r", sockNum);
			output = string(buf,GetHandleSize(p->MSG));
			
			find_and_replace(output, "\n","\r");
			
			if(wbi->toPrint == true){
				XOPNotice(report);
				XOPNotice(output.c_str());
				XOPNotice("\r");
			}
			//if there is a logfile then append and save
			if(wbi->logFile){
				GetTimeStamp(timestamp);
				(*wbi->logFile) << timestamp << "\tSEND:\t" << sockNum << "\t" << output.c_str() << endl;
			}
		/*on OSX rc<0 if remote peer is disconnected
		 on windows rc <= 0 if remote peer disconnects.  But we want to make sure that it wasn't because we tried a
		 zero length message (rc would also ==0 in that case.
		*/
		} else if (rc < 0 || (rc == 0 && GetHandleSize(p->MSG) > 0)) {
			if(wbi->toPrint == true){
				snprintf(report,sizeof(report),"SOCKIT err: problem writing to socket descriptor %d, disconnecting\r", sockNum );
				XOPNotice(report);
			}
			// Closed connection or error 
			pinstance->closeWorker(sockNum);
			err2=1;
			goto done;
		}
	} else {
		snprintf(report,sizeof(report),"SOCKIT err: timeout writing to socket %d\r", sockNum);
		if(wbi->toPrint == true)
			XOPNotice(report);
		err2=1;
		goto done;
	}
	
	//now get an immediate reply
	
	FD_ZERO(&tempset);
	timeout.tv_sec = floor(timeoutVal);
	timeout.tv_usec =  (long)((timeoutVal-(double)floor(timeoutVal))*1000000);
	FD_SET(sockNum,&tempset);
	res = select(sockNum+1,&tempset,0,0,&timeout);
	
	memset(buf,0,sizeof(buf));
	
	if ((res > 0) && FD_ISSET(sockNum, &tempset)) { 
	           
		do{			   
			rc = recvfrom(sockNum, buf, BUFLEN, 0, NULL, NULL);

			charsread += rc;
			
			//if the recv fails then the manpage indicates that rc <= 0, because we are using blocking sockets.
			if (rc <= 0) {
				snprintf(report,sizeof(report),"SOCKIT err: socket descriptor %d, disconnection???\r", sockNum );
				if(wbi->toPrint == true)
					XOPNotice(report);
				needToClose = true;
				break;
			} else if(rc > 0){
				chunk.append(buf, rc);

				if(fileToWrite)//write to file as well
					written = fwrite(buf, sizeof(char),rc, (FILE *)fileToWrite);
			}
			
			if (NBYTES_to_recv > -1 && rc >= NBYTES_to_recv)
				break;
			
			if(p->SMALFlagEncountered){
				//set a low timeout to go around again.  You only expect one packet, but you may be connected to a socket who 
				//wants to terminate after the first read(e.g. HTTP).  If it terminates the only way to pick it up is by doing a recv (<=0), but the
				//You can't just use res = 0 because otherwise the socket will never terminate.
				FD_ZERO(&tempset);
				timeout.tv_sec = 0;
				timeout.tv_usec = 10;
				FD_SET(sockNum, &tempset);
				res = select(sockNum + 1, &tempset, 0, 0, &timeout);
				//res = 0;
			} else {
				FD_ZERO(&tempset);
				timeout.tv_sec = floor(timeoutVal);
				timeout.tv_usec =  (long)((timeoutVal - (double)floor(timeoutVal))*1000000);
				FD_SET(sockNum, &tempset);
				res = select(sockNum + 1, &tempset, 0, 0, &timeout);
			}
		}while(res > 0);
		
	} else if(res == -1) {
		snprintf(report,sizeof(report),"SOCKIT err: timeout while reading socket descriptor %d, disconnecting\r", sockNum );
		if(wbi->toPrint == true)
			XOPNotice(report);
		// Closed connection or error 
		pinstance->closeWorker(sockNum);
		err2 = 1;
		goto done;
	}
	
	if(!chunk.length())
		goto done;
	
	if(p->NBYTFlagEncountered && NBYTES_to_recv > -1){
		//send excess bytes back to the buffer
		string excessData = NULL;
		if(chunk.length() - NBYTES_to_recv > 0){
			excessData = chunk.substr(NBYTES_to_recv);
			wbi->readBuffer.append(excessData);
		}
		chunk.resize(NBYTES_to_recv);
	}

	if(err = pinstance->outputBufferDataToWave(sockNum, (const unsigned char*) chunk.data(), chunk.length(), false))
		goto done;
		
done:
	if(needToClose)
		pinstance->closeWorker(sockNum);

	if(fileToWrite){
		if(XOPCloseFile(fileToWrite))
			err = PROBLEM_WRITING_TO_FILE;
	}
	if(err==0 && err2 == 0 && chunk.length()){
		if(p->retEncountered)
			err = StoreStringDataUsingVarName(p->ret,(const char*)chunk.data(),chunk.length());

		if(!err)
			SetOperationStrVar("S_tcp", (const char*) chunk.c_str());
	} else {
		if(p->retEncountered)
			err = StoreStringDataUsingVarName(p->ret,"",0);
		SetOperationStrVar("S_tcp", "");
	}
	
	if(err || err2)
		SetOperationNumVar("V_flag", 1);
	else 
		SetOperationNumVar("V_flag", 0);
	
	FD_ZERO(&tempset);

	pthread_mutex_unlock( &readThreadMutex );
	
	return err;
}

int 
SOCKITsendnrecvF(SOCKITsendnrecvFStruct *p){
	int err = 0, err2=0;
	
	extern CurrentConnections *pinstance;
	extern pthread_mutex_t readThreadMutex;
	extern bool SHOULD_IDLE_SKIP;
	pthread_mutex_lock( &readThreadMutex );


#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif

	if(!p->SMAL)
		SHOULD_IDLE_SKIP = true;
	
	string chunk;	
	Handle retval;
	
    int rc = 0;
	long charsread = 0;
	
    SOCKET sockNum = -1;
	int res = 0;
	
	char buf[BUFLEN+1];
	string output;
	long size = 0;
	waveBufferInfo *wbi = NULL;
	char timestamp[101];
		
	fd_set tempset;
	FD_ZERO(&tempset);
	
	double timeoutVal=1.;
	struct timeval timeout;
	
	retval = NewHandle(0);
	if(retval==NULL){
		err = NOMEM;
		goto done;
	}
	
	if(p->TIME){
		timeoutVal = fabs(p->TIME);
	} else {
		timeoutVal = 1.;
	}
	
	timeout.tv_sec = floor(timeoutVal);
	timeout.tv_usec =  (long)((timeoutVal-(double)floor(timeoutVal))*1000000);
	
	memset(buf, 0, sizeof(buf));
	
	// Parameter: p->MSG (test for NULL handle before using)
	if(!p->message){
		err = OH_EXPECTED_STRING;
		goto done;
	}
	size = GetHandleSize(p->message);
	if(size > BUFLEN){
		err2 = 1;
		goto done;
	}
	if(err = GetCStringFromHandle(p->message, buf, sizeof(buf)))
		goto done;
		
	if(!pinstance->isSockitOpen(p->sockID,&sockNum)){
		err2 = SOCKET_NOT_CONNECTED;
		goto done;
	}
	
	wbi = pinstance->getWaveBufferInfo(sockNum);
	
	//send the message
	FD_SET(sockNum,&tempset);
	res = select(sockNum+1, 0, &tempset, 0, &timeout);				
	if(res == -1){
		err2=1;
		goto done;
	}
	if(FD_ISSET(sockNum, &tempset)){
		rc = send(sockNum, buf, GetHandleSize(p->message),0);
		if(rc > 0){
			//if there is a logfile then append and save
			if(pinstance->getWaveBufferInfo(sockNum)->logFile){
				GetTimeStamp(timestamp);
				*(wbi->logFile) << timestamp << "\tSEND:\t" << sockNum << "\t" << buf << endl;
			}
			/*on OSX rc<0 if remote peer is disconnected
			 on windows rc <= 0 if remote peer disconnects.  But we want to make sure that it wasn't because we tried a
			 zero length message (rc would also ==0 in that case.
			 */
		} else if (rc < 0 || (rc == 0 && GetHandleSize(p->message) > 0)) {
			// Closed connection or error 
			pinstance->closeWorker(sockNum);
			err2=1;
			goto done;
		}
	} else {
		err2=1;
		goto done;
	}
	
	//now get an immediate reply
	
	FD_ZERO(&tempset);
	timeout.tv_sec = floor(timeoutVal);
	timeout.tv_usec =  (long)((timeoutVal-(double)floor(timeoutVal))*1000000);
	FD_SET(sockNum, &tempset);
	res = select(sockNum+1, &tempset, 0, 0, &timeout);
	
	memset(buf, 0, sizeof(buf));
	
	if ((res > 0) && FD_ISSET(sockNum, &tempset)){ 
		do {			   
			rc = recvfrom(sockNum, buf, BUFLEN, 0, NULL, NULL);

			charsread += rc;
			
			//if recv fails then the manpage indicates that rc <= 0, because we are using blocking sockets.
			if (rc <= 0) {
				// Closed connection or error 
				pinstance->closeWorker(sockNum);
				err2 = 1;
				break;
			} else if(rc > 0){
				chunk.append(buf, rc);
			} //else if (rc == 0)
			//	break;
			
			if(p->SMAL){
				res = 0;
			} else {
				FD_ZERO(&tempset);
				timeout.tv_sec = floor(timeoutVal);
				timeout.tv_usec =  (long)((timeoutVal-(double)floor(timeoutVal))*1000000);
				FD_SET(sockNum, &tempset);
				res = select(sockNum + 1, &tempset, 0, 0, &timeout);
			}
		} while(res > 0);
		
	} else if(res == -1) {
		// Closed connection or error 
		pinstance->closeWorker(sockNum);
		err2=1;
		goto done;
	}
	
	if(!chunk.length())
		goto done;
	
	if(wbi->logFile){
		GetTimeStamp(timestamp);
		*(wbi->logFile) << timestamp << "\tRECV:\t" << sockNum << "\t" << chunk.c_str() << endl;
	}
	
	
done:
	if(err == 0 && err2 == 0 && chunk.length())
		err = PtrAndHand((void*)chunk.data(), retval, chunk.length());
	
	p->retval = NULL;
	
	if(retval)
		p->retval = retval;
	
	FD_ZERO(&tempset);
	
	if(p->message)
		DisposeHandle(p->message);
	
	SHOULD_IDLE_SKIP = false;
	
	pthread_mutex_unlock( &readThreadMutex );
	
	return err;
};
