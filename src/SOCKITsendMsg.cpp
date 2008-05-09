#include "SOCKIT.h"


int
RegisterSOCKITsendmsg(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the SOCKITsendmsgRuntimeParams structure as well.
	cmdTemplate = "SOCKITsendmsg number:ID,string:MSG";
	runtimeNumVarList = "V_Flag";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITsendmsgRuntimeParams), (void*)ExecuteSOCKITsendmsg, 0);
}

int 
ExecuteSOCKITsendmsg(SOCKITsendmsgRuntimeParams *p){
	int err = 0, err2 = 0;
	
	extern currentConnections openConnections;
	
	#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
	#endif

    int rc = 0;
    SOCKET socketToWrite = -1;
	int res = 0;
    
	char buf[BUFLEN+1];
	char report[MAX_MSG_LEN+1];
	char *output = NULL;			//get rid of the carriage returns

	SOCKET maxSockNum = openConnections.maxSockNumber;
	
	fd_set tempset;
	FD_ZERO(&tempset);
	
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
    
	memset(buf,0,BUFLEN+1);
	memcpy(&tempset, &openConnections.readSet, sizeof(openConnections.readSet)); 

	if(!p->MSGEncountered){
		err = OH_EXPECTED_STRING;
		goto done;
	} else {
		if(err = GetCStringFromHandle(p->MSG, buf, sizeof(buf)))
			goto done;
	}
	
	if(!p->IDEncountered){
		err = OH_EXPECTED_NUMBER;
		goto done;
	}

	socketToWrite = (SOCKET)p->ID;
	
	if (socketToWrite <= 0) {
		snprintf(report,sizeof(report),"SOCKIT err: socket not connected %d\r", socketToWrite);
		XOPNotice(report);
		err2 = 1;
		goto done;
	} else {
		if(!FD_ISSET(socketToWrite,&tempset)){
			snprintf(report,sizeof(report),"SOCKIT err: can't write to socket %d\r", socketToWrite);
			XOPNotice(report);
			err2 = 1;
			goto done;
		}
	}
	
	//flush messages first
	err = checkRecvData();
	
	res = select(maxSockNum+1,0,&tempset,0,&timeout);
	if(res == -1){
		XOPNotice ("SOCKIT err: select returned -1");
		err2 = 1;
        goto done;
	}
	if(FD_ISSET(socketToWrite,&tempset)){
		rc = send(socketToWrite,buf,strlen(buf),0);
		if(rc >= 0 && openConnections.bufferWaves[socketToWrite].toPrint == true){
			snprintf(report,sizeof(report),"SOCKITmsg: wrote to socket %d\r", socketToWrite);
			XOPNotice(report);
			output = NtoCR(buf, "\n","\r");
			XOPNotice(output);
			XOPNotice("\r");
			goto done;
		} else if (rc < 0) {
			snprintf(report,sizeof(report),"SOCKIT err: problem writing to socket descriptor %d, disconnecting\r", socketToWrite );
			XOPNotice(report);
			// Closed connection or error 
			SOCKITcloseWorker(socketToWrite);
			err2 = 1;
		}
	} else {
		snprintf(report,sizeof(report),"SOCKIT err: timeout writing to socket %d\r", socketToWrite);
		XOPNotice(report);
		err2 = 1;
		goto done;
	}
		
done:
	if(output!= NULL)
		free(output);
	
	if(err || err2){
		SetOperationNumVar("V_flag", 1);
	} else {
		SetOperationNumVar("V_flag", 0);
	}
		
	FD_ZERO(&tempset);

return err;
}