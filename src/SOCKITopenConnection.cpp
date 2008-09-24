#include "SOCKIT.h"
#include "SOCKITopenconnection.h"

int
RegisterSOCKITopenconnection(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the SOCKITopenconnectionRuntimeParams structure as well.
	cmdTemplate = "SOCKITopenconnection/TIME=number/LOG=name/DBUG/Q[=number]/Tok=string/proc=name varname:ID,string:IP,number:PORT,wave:BUF";
	runtimeNumVarList = "V_flag";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITopenconnectionRuntimeParams), (void*)ExecuteSOCKITopenconnection, 0);
}

static int
ExecuteSOCKITopenconnection(SOCKITopenconnectionRuntimeParamsPtr p)
{
	int err = 0, err2 = 0;
	
	extern CurrentConnections* pinstance;
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

	fd_set tempset;	
	struct timeval timeout;
	
	struct sockaddr_in  sa;
    struct hostent*     hen;
	unsigned long fdflags;
	waveBufferInfo *bufferInfo = new waveBufferInfo();
	
	xmlNode *root_element = NULL;
	xmlChar *entityEncoded = NULL;
	char fnamepath[MAX_PATH_LEN+1];
	char nativepath[MAX_PATH_LEN+1];
	char fname[MAX_FILENAME_LEN+1];
	char a[10];
	long year,month,day,hour,minute,second;
	
	memset(fnamepath,0,sizeof(fnamepath));
	if(p->TIMEFlagEncountered){
		timeout.tv_sec = floor(p->TIMEFlagNumber);
		timeout.tv_usec =  (int)(p->TIMEFlagNumber-(double)floor(p->TIMEFlagNumber))*1000000;
	} else {
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
	}
	
	if(p->IDEncountered){
		if(err = VarNameToDataType(p->IDVarName, &dataType)) 
			goto done;
		if(dataType != NT_FP64){
			err = EXPECTED_NUM_VAR_OR_NVAR;
			goto done;
		}
	}
	
	if(p->LOGFlagEncountered){
		memset(fnamepath,0, sizeof(fnamepath));
		memset(a,0,sizeof(a));
		snprintf(a,9,"a.xml");

		if(err = GetFullPathFromSymbolicPathAndFilePath(p->LOGFlagName,a,fnamepath))
			goto done;
		if(err = GetDirectoryAndFileNameFromFullPath(fnamepath, fnamepath, a))
			goto done;
			
		if(FullPathPointsToFolder(fnamepath)){
			GetTheTime(&year,&month,&day,&hour,&minute,&second);
			snprintf(fname, MAX_FILENAME_LEN, "log%02ld%02ld%02ld%d%d%d.xml",year,month,day,hour,minute,second);
			if(err = ConcatenatePaths(fnamepath,fname,fnamepath))
				goto done;
			if(err = GetNativePath(fnamepath,nativepath))
				goto done;
			if(err = XOPOpenFile(nativepath,1,&bufferInfo->logFile))
				goto done;
		}
	}
	
	// Flag parameters.
	if (p->DBUGFlagEncountered) {
		bufferInfo->DBUG = true;
	} else {
		bufferInfo->DBUG = false;
	}
	
	if (p->QFlagEncountered) {
		bufferInfo->toPrint = false;

		if (p->QFlagParamsSet[0]) {
			// Optional parameter: p->QFlagNumber
			if( (int)p->QFlagNumber == 1)
				bufferInfo->toPrint = true;
		}
	} else {
		bufferInfo->toPrint = true;
	}
	
	if (p->TOKFlagEncountered) {
		// Parameter: p->TOKFlagStrH (test for NULL handle before using)
		if(!p->TOKFlagStrH){
			err = OH_EXPECTED_STRING;
			goto done;
		}
		GetCStringFromHandle(p->TOKFlagStrH,bufferInfo->tokenizer,30);
		//we don't use strlen because we're interested in 0x00
		//that would normally terminate a string.
		bufferInfo->sztokenizer = GetHandleSize(p->TOKFlagStrH);		
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
		bufferInfo->bufferWave = p->BUFWaveH;
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
		if(!p->QFlagEncountered)
			XOPNotice("SOCKITmsg: Failed to create new socket\r");
		goto done;
	}
	
	FD_ZERO(&tempset);
	FD_SET(sockNum,&tempset);
	
	/* Connect to server */
#ifdef _MACINTOSH_
	fdflags = fcntl(sockNum, F_GETFL);
	if(fcntl(sockNum, F_SETFL, fdflags | O_NONBLOCK) < 0){
		err2 = 1;
		XOPNotice("SOCKITerr: fcntl failed\r");
		goto done;
	}
#endif
#ifdef _WINDOWS_
	fdflags = 1;
	if(err2 = ioctlsocket(sockNum, FIONBIO, &fdflags)){
		XOPNotice("SOCKITerr: IOCTL failed \r");
		goto done;
	}
#endif

	rc = connect(sockNum, (struct sockaddr *)&sa, sizeof(sa));
	res = select(sockNum+1,0,&tempset,0,&timeout);
	
	if(FD_ISSET(sockNum, &tempset)){
		if(!p->QFlagEncountered){
				snprintf(report,sizeof(report),"SOCKITmsg: Connected %s as socket number %d\r", host, sockNum );
				XOPNotice(report);
		}
	} else {
		if(!p->QFlagEncountered)
			XOPNotice("SOCKIT err: couldn't make new connection\r");
		close(sockNum);
		sockNum = -1;
		err2 = 1;
		goto done;
	}

//reset to blocking
#ifdef _MACINTOSH_
	fdflags = fcntl(sockNum, F_GETFL);
	if(fcntl(sockNum, F_SETFL, fdflags | (~O_NONBLOCK)) < 0){
		err2 = 1;
		XOPNotice("SOCKITerr:fcntl failed\r");
		goto done;
	}
#endif
#ifdef _WINDOWS_
	fdflags = 0;
	if(err2 = ioctlsocket(sockNum, FIONBIO, &fdflags)){
		XOPNotice("SOCKITerr: IOCTL failed \r");
		goto done;
	}
#endif

	//socket succeeded in connecting, add to the map containing all the open connections, connect a processor
	if(sockNum>0){
		if(strlen(fnamepath) > 0){
			bufferInfo->logDoc = xmlNewDoc(BAD_CAST "1.0");
			if(bufferInfo->logDoc == NULL){
				if(!p->QFlagEncountered)
					XOPNotice("SOCKIT err: couldn't create logfile)\r");
				goto done;
			}
			//create the root element
			root_element=xmlNewNode(NULL , BAD_CAST "SOCKIT");
			if(root_element == NULL){
				if(!p->QFlagEncountered)
					XOPNotice("SOCKIT err: couldn't create logfile)\r");
				goto done;
			}
			entityEncoded = xmlEncodeEntitiesReentrant(bufferInfo->logDoc, BAD_CAST host);
			
			xmlSetProp(root_element, BAD_CAST "IP", entityEncoded);
			snprintf(report, sizeof(report), "%d",port);
			if(entityEncoded != NULL){
				xmlFree(entityEncoded);
				entityEncoded = NULL;
			}
			entityEncoded = xmlEncodeEntitiesReentrant(bufferInfo->logDoc, BAD_CAST report);
			xmlSetProp(root_element, BAD_CAST "port", BAD_CAST xmlEncodeEntitiesReentrant(bufferInfo->logDoc, BAD_CAST entityEncoded));		
			root_element = xmlDocSetRootElement(bufferInfo->logDoc,root_element);
		}
		pinstance->addWorker(sockNum,*bufferInfo);
		
		if (p->PROCFlagEncountered) {
			// Parameter: p->PROCFlagName
			if(err = pinstance->registerProcessor(sockNum,p->PROCFlagName)){
				err = PROCESSOR_NOT_AVAILABLE;
				if(!p->QFlagEncountered)
					XOPNotice("SOCKIT err: processor must be f(textWave,variable)\r");
				goto done;
			}
		} else {
			pinstance->registerProcessor(sockNum,"");
		}
		
	}
	
done:
	FD_ZERO(&tempset);
	if(!err && sockNum>0 && !err2){
		err = SetOperationNumVar("V_flag",0);
		err = StoreNumericDataUsingVarName(p->IDVarName,sockNum,0);
	} else {
		delete bufferInfo;
		err = SetOperationNumVar("V_flag",1);
		err = StoreNumericDataUsingVarName(p->IDVarName,-1,0);
	}
	
	if(entityEncoded!= NULL)
		xmlFree(entityEncoded);
	
	return err;
}
