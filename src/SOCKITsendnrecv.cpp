#include "SOCKIT.h"
#include "SOCKITsendnrecv.h"

int
RegisterSOCKITsendnrecv(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the SOCKITopenconnectionRuntimeParams structure as well.
	cmdTemplate = "SOCKITsendnrecv/FILE=string/TIME=number/SMAL number:ID,string:MSG [,varname:ret]";
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
	string output;
	long size = 0;
	bool needToClose = false;
	
	xmlNode *added_node = NULL;
	xmlNode *root_element = NULL;
	xmlChar *encContent = NULL;
	long year,month,day,hour,minute,second;
	char timebuf[100];
		
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
			
			if( pinstance->getWaveBufferInfo(sockNum)->toPrint == true){
				XOPNotice(report);
				XOPNotice(output.c_str());
				XOPNotice("\r");
			}
			//if there is a logfile then append and save
			if(pinstance->getWaveBufferInfo(sockNum)->logDoc != NULL){
				root_element = xmlDocGetRootElement(pinstance->getWaveBufferInfo(sockNum)->logDoc);
				encContent = xmlEncodeEntitiesReentrant(pinstance->getWaveBufferInfo(sockNum)->logDoc, BAD_CAST output.c_str());
				added_node = xmlNewChild(root_element, NULL, BAD_CAST "SEND" ,encContent);
				if(encContent != NULL){
					xmlFree(encContent);
					encContent = NULL;
				}

				GetTheTime(&year,&month,&day,&hour,&minute,&second);
				snprintf(timebuf, 99, "%02ld/%02ld/%02ld %02ld:%02ld:%02ld",year,month,day,hour,minute,second);

				encContent = xmlEncodeEntitiesReentrant(pinstance->getWaveBufferInfo(sockNum)->logDoc, BAD_CAST timebuf);
				xmlSetProp(added_node, BAD_CAST "time", encContent);
				
				rewind((pinstance->getWaveBufferInfo(sockNum)->logFile));
				if(xmlDocFormatDump((pinstance->getWaveBufferInfo(sockNum)->logFile),pinstance->getWaveBufferInfo(sockNum)->logDoc,0)==-1){
					XOPCloseFile((pinstance->getWaveBufferInfo(sockNum)->logFile));
				}
			}
		/*on OSX rc<0 if remote peer is disconnected
		 on windows rc <= 0 if remote peer disconnects.  But we want to make sure that it wasn't because we tried a
		 zero length message (rc would also ==0 in that case.
		*/
		} else if (rc < 0 || (rc == 0 && GetHandleSize(p->MSG) > 0)) {
			if(pinstance->getWaveBufferInfo(sockNum)->toPrint == true){
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
		if(pinstance->getWaveBufferInfo(sockNum)->toPrint == true)
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
			rc = recv(sockNum, buf, BUFLEN, 0);

			charsread += rc;
			
			//if the recv fails then the manpage indicates that rc <= 0, because we are using blocking sockets.
			if (rc <= 0) {
				snprintf(report,sizeof(report),"SOCKIT err: socket descriptor %d, disconnection???\r", sockNum );
				if(pinstance->getWaveBufferInfo(sockNum)->toPrint == true)
					XOPNotice(report);
				needToClose = true;
				break;
			} else if(rc > 0){
				if(chunk.append(buf, sizeof(char), rc) == -1){
					err = NOMEM;
					goto done;
				}
				if(fileToWrite)//write to file as well
					written = fwrite(buf, sizeof(char),rc, (FILE *)fileToWrite);
			}// else if (rc == 0)
			//	break;
			
			if(p->SMALFlagEncountered){
				res = 0;
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
		if(pinstance->getWaveBufferInfo(sockNum)->toPrint == true)
			XOPNotice(report);
		// Closed connection or error 
		pinstance->closeWorker(sockNum);
		err2 = 1;
		goto done;
	}
	
	if(!chunk.getData())
		goto done;
	
	if(err = pinstance->outputBufferDataToWave(sockNum, chunk.getData(), chunk.getMemSize(), false))
		goto done;
	

done:
	if(needToClose)
		pinstance->closeWorker(sockNum);

	if(fileToWrite){
		if(XOPCloseFile(fileToWrite))
			err = PROBLEM_WRITING_TO_FILE;
	}
	if(err==0 && err2 == 0 && chunk.getData()){
		if(p->retEncountered)
			err = StoreStringDataUsingVarName(p->ret,(const char*)chunk.getData(),chunk.getMemSize());

		try {
			chunk.nullTerminate();
		} catch (bad_alloc&) {
			err = NOMEM;
		}

		if(!err)
			SetOperationStrVar("S_tcp", (const char*) chunk.getData());
	} else {
		if(p->retEncountered)
			err = StoreStringDataUsingVarName(p->ret,"",0);
		SetOperationStrVar("S_tcp", "");
	}
	
	if(err || err2)
		SetOperationNumVar("V_flag", 1);
	else 
		SetOperationNumVar("V_flag", 0);

	if(encContent != NULL)
		xmlFree(encContent);
	
	FD_ZERO(&tempset);

	pthread_mutex_unlock( &readThreadMutex );
	
	return err;
}

int 
SOCKITsendnrecvF(SOCKITsendnrecvFStruct *p){
	int err = 0, err2=0;
	
	extern CurrentConnections *pinstance;
	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );


#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif
	
	MemoryStruct chunk;	
	Handle retval;
	
    int rc = 0;
	long charsread = 0;
	
    SOCKET sockNum = -1;
	int res = 0;
	
	char buf[BUFLEN+1];
	string output;
	long size = 0;
	
	xmlNode *added_node = NULL;
	xmlNode *root_element = NULL;
	xmlChar *encContent = NULL;
	long year, month, day, hour, minute, second;
	char timebuf[100];
		
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
			if(pinstance->getWaveBufferInfo(sockNum)->logDoc != NULL){
				root_element = xmlDocGetRootElement(pinstance->getWaveBufferInfo(sockNum)->logDoc);
				encContent = xmlEncodeEntitiesReentrant(pinstance->getWaveBufferInfo(sockNum)->logDoc, BAD_CAST output.c_str());
				added_node = xmlNewChild(root_element, NULL, BAD_CAST "SEND" ,encContent);
				if(encContent != NULL){
					xmlFree(encContent);
					encContent = NULL;
				}

				GetTheTime(&year,&month,&day,&hour,&minute,&second);
				snprintf(timebuf, 99, "%02ld/%02ld/%02ld %02ld:%02ld:%02ld",year,month,day,hour,minute,second);

				encContent = xmlEncodeEntitiesReentrant(pinstance->getWaveBufferInfo(sockNum)->logDoc, BAD_CAST timebuf);
				xmlSetProp(added_node, BAD_CAST "time", encContent);
				
				rewind((pinstance->getWaveBufferInfo(sockNum)->logFile));
				if(xmlDocFormatDump((pinstance->getWaveBufferInfo(sockNum)->logFile),pinstance->getWaveBufferInfo(sockNum)->logDoc,0)==-1){
					XOPCloseFile((pinstance->getWaveBufferInfo(sockNum)->logFile));
				}
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
			rc = recv(sockNum, buf, BUFLEN, 0);

			charsread += rc;
			
			//if recv fails then the manpage indicates that rc <= 0, because we are using blocking sockets.
			if (rc <= 0) {
				// Closed connection or error 
				pinstance->closeWorker(sockNum);
				err2 = 1;
				break;
			} else if(rc > 0){
				if(chunk.append(buf, sizeof(char), rc) == -1){
					err = NOMEM;
					goto done;
				}
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
	
	if(!chunk.getData()){
		goto done;
	} 
	
done:
	if(err == 0 && err2 == 0 && chunk.getData())
		err = PtrAndHand((void*)chunk.getData(), retval, chunk.getMemSize());
	
	p->retval = NULL;
	
	if(retval)
		p->retval = retval;
	
	if(encContent != NULL)
		xmlFree(encContent);
	
	FD_ZERO(&tempset);
	
	if(p->message)
		DisposeHandle(p->message);
	
	pthread_mutex_unlock( &readThreadMutex );
	
	return err;
};
