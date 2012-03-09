#include "CurrentConnections.h"
#include "SOCKITsendmsg.h"

int
RegisterSOCKITsendmsg(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the SOCKITsendmsgRuntimeParams structure as well.
	cmdTemplate = "SOCKITsendmsg/TIME=number number:ID,string:MSG";
	runtimeNumVarList = "V_Flag";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITsendmsgRuntimeParams), (void*)ExecuteSOCKITsendmsg, 0);
}

int 
ExecuteSOCKITsendmsg(SOCKITsendmsgRuntimeParams *p){
	int err = 0, err2 = 0;
	
	extern CurrentConnections *pinstance;
	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
			
#ifdef WINIGOR
	extern WSADATA globalWsaData;
#endif
	
    int rc = 0;
    SOCKET socketToWrite = -1;
	int res = 0;
    
	char buf[BUFLEN+1];
	char report[MAX_MSG_LEN+1];
	string output;			//get rid of the carriage returns
	waveBufferInfo *wbi = NULL;
	long size = 0;
	fd_set tempset;
	struct timeval timeout;
	
	memset(buf,0,sizeof(buf));
	
	if(p->TIMEFlagEncountered){
		timeout.tv_sec = (long) floor(p->TIMEFlagNumber);
		timeout.tv_usec =  (long)((p->TIMEFlagNumber-(double)floor(p->TIMEFlagNumber))*1000000);		
	} else {
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
    }
	
	if(!p->MSGEncountered){
		err = OH_EXPECTED_STRING;
		goto done;
	} else {
		size = GetHandleSize(p->MSG);
		if(size>BUFLEN){
			err2 = 1;
			XOPNotice("SOCKIT err: message is longer than buffer\r");
			goto done;
		}
		if(err = GetCStringFromHandle(p->MSG, buf, sizeof(buf)))
			goto done;
	}
	
	if(!p->IDEncountered){
		err = OH_EXPECTED_NUMBER;
		goto done;
	}
	
	if(!pinstance->isSockitOpen(p->ID,&socketToWrite)){
		snprintf(report,sizeof(report),"SOCKIT err: socket not connected %ld\r", socketToWrite);
		XOPNotice(report);
		err2 = 1;
		goto done;
	}
	
	wbi = pinstance->getWaveBufferInfo(socketToWrite);
	
	FD_ZERO(&tempset);
	FD_SET(socketToWrite, &tempset);
	
	res = select(socketToWrite+1, 0, &tempset, 0, &timeout);
	if(res == -1){
		if(wbi->toPrint == true)
			XOPNotice ("SOCKIT err: select returned -1");
		err2 = 1;
        goto done;
	}
	if(FD_ISSET(socketToWrite,&tempset)){
		rc = send(socketToWrite, buf, GetHandleSize(p->MSG), 0);
		if(rc > 0){
			snprintf(report,sizeof(report), "SOCKITmsg: wrote to socket %d\r", socketToWrite);
			output = string(buf, GetHandleSize(p->MSG));
			find_and_replace(output, "\n", "\r");
			
			if(wbi->toPrint == true){
				XOPNotice(report);
				XOPNotice(output.c_str());
				XOPNotice("\r");
			}			
			//if there is a logfile then append and save
			wbi->log_msg(output.c_str(), 1);
							
		/*on OSX rc<0 if remote peer is disconnected
		  on windows rc <= 0 if remote peer disconnects.  But we want to make sure that it wasn't because we tried a
		 zero length message (rc would also ==0 in that case.
		 */
		} else if (rc < 0 || (rc == 0 && GetHandleSize(p->MSG) > 0)) {// Closed connection or error
			snprintf(report,sizeof(report),"SOCKIT err: problem writing to socket descriptor %d, disconnecting\r", socketToWrite );
			if(wbi->toPrint == true)
				XOPNotice(report);
			pinstance->closeWorker(socketToWrite);
			err2 = 1;
		}
	} else {
			snprintf(report,sizeof(report),"SOCKIT err: timeout writing to socket %d\r", socketToWrite);
			if(wbi->toPrint == true)
				XOPNotice(report);
		err2 = 1;
		goto done;
	}
	
done:
	
	if(err || err2){
		SetOperationNumVar("V_flag", 1);
	} else {
		SetOperationNumVar("V_flag", 0);
	}
	
	FD_ZERO(&tempset);

	pthread_mutex_unlock( &readThreadMutex );

	return err;
}

int 
SOCKITsendmsgF(SOCKITsendmsgFStruct *p){
	int err = 0, err2 = 0;
	
	extern CurrentConnections *pinstance;
	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );

#ifdef WINIGOR
	extern WSADATA globalWsaData;
#endif
	
    int rc = 0;
    SOCKET socketToWrite = -1;
	int res = 0;
    
	char buf[BUFLEN+1];
	string output;			//get rid of the carriage returns
	waveBufferInfo *wbi = NULL;
	long size = 0;
	fd_set tempset;
	struct timeval timeout;
	
	memset(buf,0,sizeof(buf));

	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	
	if(!p->message){
		err = OH_EXPECTED_STRING;
		goto done;
	} else {
		size = GetHandleSize(p->message);
		if(size>BUFLEN){
			err2 = 1;
			goto done;
		}
		if(err = GetCStringFromHandle(p->message, buf, sizeof(buf)))
			goto done;
	}
	
	if(!p->sockID){
		err = OH_EXPECTED_NUMBER;
		goto done;
	}
	
	if(!pinstance->isSockitOpen(p->sockID,&socketToWrite)){
		err2 = 1;
		goto done;
	}
	
	wbi = pinstance->getWaveBufferInfo(socketToWrite);
	
	FD_ZERO(&tempset);
	FD_SET(socketToWrite, &tempset);
	
	res = select(socketToWrite+1, 0, &tempset, 0, &timeout);
	if(res == -1){
		err2 = 1;
        goto done;
	}
	if(FD_ISSET(socketToWrite,&tempset)){
		rc = send(socketToWrite, buf, GetHandleSize(p->message), 0);
		if(rc > 0){
			//if there is a logfile then append and save
			wbi->log_msg(output.c_str(), 1);
			
			goto done;
			/*on OSX rc<0 if remote peer is disconnected
			 on windows rc <= 0 if remote peer disconnects.  But we want to make sure that it wasn't because we tried a
			 zero length message (rc would also ==0 in that case.
			 */
		} else if (rc < 0 || (rc == 0 && GetHandleSize(p->message) > 0)) {
			pinstance->closeWorker(socketToWrite);
			err2 = 1;
		}
	} else {
		err2 = 1;
		goto done;
	}
	
done:
	
	if(err || err2){
		p->retval = 1;
	} else {
		p->retval = 0;
	}
	
	FD_ZERO(&tempset);
	
	if(p->message)
		DisposeHandle(p->message);
					
	pthread_mutex_unlock( &readThreadMutex );

	return err;
}

