#include "SOCKIT.h"

int
RegisterSOCKITopenconnection(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the SOCKITopenconnectionRuntimeParams structure as well.
	cmdTemplate = "SOCKITopenconnection/Q/Tok=string/proc=name varname:ID,string:IP,number:PORT,wave:BUF";
	runtimeNumVarList = "";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITopenconnectionRuntimeParams), (void*)ExecuteSOCKITopenconnection, 0);
}

static int
ExecuteSOCKITopenconnection(SOCKITopenconnectionRuntimeParamsPtr p)
{
	int err = 0;
	
	extern currentConnections openConnections;
#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif
	
	int rc;
    SOCKET sockNum = -1;
	int res = 0;
	long port = 0;
	int dataType = 0;
	
	char host[MAX_URL_LEN+1];
	char report[MAX_MSG_LEN+1];
	SOCKET maxSockNum = openConnections.maxSockNumber;
	
	fd_set tempset;
	FD_ZERO(&tempset);
	
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	
	struct sockaddr_in  sa;
    struct hostent*     hen;
    struct waveBufferInfoStruct bufferInfoStruct;
	
	memcpy(&tempset, &openConnections.readSet, sizeof(openConnections.readSet)); 
	
	if(p->IDEncountered){
		if(err = VarNameToDataType(p->IDVarName, &dataType)) 
			goto done;
		if(dataType != NT_FP64){
			err = EXPECTED_NUM_VAR_OR_NVAR;
			goto done;
		}
	}
	
	// Flag parameters.
	if (p->QFlagEncountered) {
		bufferInfoStruct.toPrint = false;
	} else {
		bufferInfoStruct.toPrint = true;
	}
	
	if (p->TOKFlagEncountered) {
		// Parameter: p->TOKFlagStrH (test for NULL handle before using)
		if(!p->TOKFlagStrH){
			err = OH_EXPECTED_STRING;
			goto done;
		}
		if(err = GetCStringFromHandle(p->TOKFlagStrH,bufferInfoStruct.tokenizer,10))
			goto done;
	} else {
		memset(bufferInfoStruct.tokenizer,0,sizeof(bufferInfoStruct.tokenizer));
	}
	
	if (p->PORTEncountered) {
		// Parameter: p->PORTNumber
		port = (long)p->PORTNumber;
	}
	
	if (p->BUFEncountered) {
		// Parameter: p->BUFWaveH (test for NULL handle before using)
		if(!p->BUFWaveH || (WaveType(p->BUFWaveH) != TEXT_WAVE_TYPE)){
			err = EXPECTED_TEXT_WAVE;
			goto done;
		}
		bufferInfoStruct.bufferWave = p->BUFWaveH;
	}
	
	if(p->IPEncountered){
		// Parameter: p->IPStrH (test for NULL handle before using)
		if(!p->IPStrH){
			err = OH_EXPECTED_STRING;
			goto done;
		}
		if(err = GetCStringFromHandle(p->IPStrH, host, MAX_URL_LEN))
			goto done;
	}
	
	
	/* Address resolution */
	hen = gethostbyname(host);
	if (!hen){
		err = BAD_HOST_RESOLV;
		goto done;
	}
	
	/* zero internet address struct*/
	memset(&sa, 0, sizeof(sa));
	
	/* populate internet address struct */
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	memcpy(&sa.sin_addr.s_addr, hen->h_addr_list[0], hen->h_length);
	
	/* allocate a socket */
	sockNum = socket(AF_INET, SOCK_STREAM, 0);
	if (sockNum < 0) {
		XOPNotice("SOCKITmsg: Failed to create new socket\r");
		goto done;
	}
	
	FD_SET(sockNum,&tempset);
	if(sockNum > openConnections.maxSockNumber){
		maxSockNum = sockNum;
	}
	
	/* Connect to server */
	
	rc = connect(sockNum, (struct sockaddr *)&sa, sizeof(sa));
	res = select(maxSockNum+1,0,&tempset,0,&timeout);
	
	if(FD_ISSET(sockNum, &tempset)){
		if(rc==0){
			FD_SET(sockNum,&openConnections.readSet);
			if(sockNum > openConnections.maxSockNumber){
				openConnections.maxSockNumber = sockNum;
			}
			snprintf(report,sizeof(report),"SOCKITmsg: Connected %s as socket number %d\r", host, sockNum );
			XOPNotice(report);
		} else {
			perror("Error failed because:");
			XOPNotice("SOCKIT err: failed to connect\r");
			SOCKITcloseWorker(sockNum);
			sockNum = -1;
			goto done;
		}
	} else {
		if( rc !=0){
			perror("Error failed because:");
			XOPNotice("SOCKIT err: timeout while waiting for new connection\r");
			SOCKITcloseWorker(sockNum);
			sockNum = -1;
			goto done;
		}
	}
	
	
	//socket succeeded in connecting, add to the map containing all the open connections, connect a processor
	if(sockNum>0){
		openConnections.bufferWaves[sockNum] = bufferInfoStruct;
		
		if (p->PROCFlagEncountered) {
			// Parameter: p->PROCFlagName
			if(err = registerProcessor(sockNum, p->PROCFlagName)){
				err = PROCESSOR_NOT_AVAILABLE;
				goto done;
			}
		} else {
			registerProcessor(sockNum, "");
		}
	}
	
done:
	FD_ZERO(&tempset);
	if(!err && sockNum>0){
		err = SetOperationNumVar("V_flag",0);
		err = StoreNumericDataUsingVarName(p->IDVarName,sockNum,0);
	} else {
		err = SetOperationNumVar("V_flag",1);
		err = StoreNumericDataUsingVarName(p->IDVarName,-1,0);
	}
	
	return err;
}
