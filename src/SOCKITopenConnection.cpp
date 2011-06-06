#include "SOCKIT.h"
#include "SOCKITopenconnection.h"

int
RegisterSOCKITopenconnection(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the SOCKITopenconnectionRuntimeParams structure as well.
	cmdTemplate = "SOCKITopenconnection/NOID/TIME=number/LOG=name/DBUG/Q[=number]/Tok=string/proc=name varname:ID,string:IP,number:PORT,wave:BUF";
	runtimeNumVarList = "V_flag";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITopenconnectionRuntimeParams), (void*)ExecuteSOCKITopenconnection, 0);
}

static int
ExecuteSOCKITopenconnection(SOCKITopenconnectionRuntimeParamsPtr p)
{
	int err = 0, err2 = 0;
	
	extern CurrentConnections* pinstance;
	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
	
#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif
	
	int rc;
    SOCKET sockNum = -1L;
	int res = 0;
	char port[7];
	int dataType = 0;
	unsigned long fdflags;
	int ignoreSIGPIPE = 1;
	
	char host[MAX_URL_LEN+1];
	char report[MAX_MSG_LEN+1];

	fd_set tempset;	
	struct timeval timeout;
	
	struct addrinfo hints, *servinfo = NULL;
	
	waveBufferInfo *bufferInfo = NULL;
		
	char fnamepath[MAX_PATH_LEN + 1];
	char nativepath[MAX_PATH_LEN + 1];
	char fname[MAX_FILENAME_LEN + 1];
	char a[10];
	
	memset(fnamepath, 0, sizeof(char) * (MAX_PATH_LEN + 1));
	memset(report, 0, sizeof(char) * (MAX_MSG_LEN + 1));
	
	if(p->TIMEFlagEncountered){
		timeout.tv_sec = floor(p->TIMEFlagNumber);
		timeout.tv_usec =  (long)((p->TIMEFlagNumber-(double)floor(p->TIMEFlagNumber))*1000000);
	} else {
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
	}
	
	bufferInfo = new waveBufferInfo();
	if(bufferInfo == NULL){
		err = NOMEM;
		goto done;
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
		memset(fnamepath, 0, sizeof(fnamepath));
		memset(a, 0, sizeof(a));
		snprintf(a, 9, "log.txt");

		if(err = GetFullPathFromSymbolicPathAndFilePath(p->LOGFlagName, a, fnamepath))
			goto done;
		if(err = GetDirectoryAndFileNameFromFullPath(fnamepath, fnamepath, a))
			goto done;
			
		if(FullPathPointsToFolder(fnamepath)){
			if(err = ConcatenatePaths(fnamepath,"log.txt",fnamepath))
				goto done;
			if(err = GetNativePath(fnamepath,nativepath))
				goto done;
		}
		//TODO
	}
	
	// Flag parameters.
	if (p->DBUGFlagEncountered) {
		bufferInfo->DBUG = true;
	} else {
		bufferInfo->DBUG = false;
	}
	
	if(p->NOIDFlagEncountered){
		bufferInfo->NOIDLES = true;
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
		memcpy(bufferInfo->hostIP, host, sizeof(char) * strlen(host));
	}
	
	
	/* Address resolution */	
	memset(&hints, 0, sizeof(hints));
	memset(port, 0, sizeof(char) * 7);
	
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	snprintf(port, 6, "%d", (long)p->PORTNumber);
	
	if(getaddrinfo(host, port, &hints, &servinfo)){
		err = BAD_HOST_RESOLV;
		goto done;
	}
	
	/* allocate a socket */
	sockNum = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (sockNum < 0) {
		if(!p->QFlagEncountered)
			XOPNotice("SOCKITmsg: Failed to create new socket\r");
		goto done;
	}

	FD_ZERO(&tempset);
	FD_SET(sockNum, &tempset);
	
	/* Connect to server */
#ifdef _MACINTOSH_
	fdflags = fcntl(sockNum, F_GETFL, 0);
	fdflags |= O_NONBLOCK;
	fcntl(sockNum, F_SETFL, fdflags);
	/*if you don't ignore SIGPIPE then IGOR could crash if the other side exits prematurely.
	this is for the case that the connect() operation works, but when you try to send()
	to it there is nothing connected (or the other end has exited.
	in this case SIGPIPE is issued and terminates the thread.  We don't want that, we'll
	just rely on errno = EPIPE.*/
	setsockopt(sockNum, SOL_SOCKET, SO_NOSIGPIPE, (void *)&ignoreSIGPIPE, sizeof(int));
#endif
	
#ifdef _WINDOWS_
	fdflags = 1;
	if(err2 = ioctlsocket(sockNum, FIONBIO, &fdflags)){
		XOPNotice("SOCKITerr: IOCTL failed \r");
		goto done;
	}
#endif

	rc = connect(sockNum, servinfo->ai_addr, servinfo->ai_addrlen);
	res = select(sockNum + 1, 0, &tempset,0, &timeout);

	if(res > 0 && FD_ISSET(sockNum, &tempset)){
		if(!p->QFlagEncountered){
				snprintf(report, sizeof(char) * MAX_MSG_LEN, "SOCKITmsg: Connected %s as socket number %d\r", host, sockNum );
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
	fdflags = fcntl(sockNum, F_GETFL, 0);
	fdflags &= ~(O_NONBLOCK);
	res = fcntl(sockNum, F_SETFL, fdflags);
#endif
	
#ifdef _WINDOWS_
	fdflags = 0;
	if(err2 = ioctlsocket(sockNum, FIONBIO, &fdflags)){
		XOPNotice("SOCKITerr: IOCTL failed \r");
		goto done;
	}
#endif

	//socket succeeded in connecting, add to the map containing all the open connections, connect a processor
	if(sockNum > 0){
		if(strlen(fnamepath) > 0)
			memcpy(bufferInfo->logFileName, fnamepath, sizeof(char) * strlen(fnamepath));

		if(err = pinstance->addWorker(sockNum, *bufferInfo)){
			err = SOCKET_ALLOC_FAILED;
			goto done;
		}
		
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
	if(servinfo)
		freeaddrinfo(servinfo);
	
	FD_ZERO(&tempset);
	if(!err && sockNum>0 && !err2){
		err = SetOperationNumVar("V_flag", 0);
		err = StoreNumericDataUsingVarName(p->IDVarName, sockNum,0);
	} else {
		err = SetOperationNumVar("V_flag",1);
		err = StoreNumericDataUsingVarName(p->IDVarName, -1, 0);
	}
	
	if(bufferInfo)
		delete bufferInfo;
	
	pthread_mutex_unlock( &readThreadMutex );
	
	
	return err;
}

int
SOCKITopenconnectionF(SOCKITopenconnectionFStructPtr p)
{
	int err = 0, err2 = 0;
	
	extern CurrentConnections* pinstance;
	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
	
	p->retval=-1;
	
#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif
	
	int rc;
    SOCKET sockNum = -1;
	int res = 0;
	long port = 0;
	unsigned long fdflags;
	int ignoreSIGPIPE = 1;
	
	char host[MAX_URL_LEN+1];

	fd_set tempset;	
	struct timeval timeout;
	
	struct sockaddr_in  sa;
    struct hostent     *hen;
	waveBufferInfo *bufferInfo = new waveBufferInfo();
		
	timeout.tv_sec = floor(p->timeout);
	timeout.tv_usec =  (long)((p->timeout-(double)floor(p->timeout))*1000000);
	
	bufferInfo->NOIDLES = true;
	bufferInfo->toPrint = false;
	
	port = (long)p->portNumber;
		
	if(!p->IPStr){
		err = OH_EXPECTED_STRING;
		goto done;
	}
	if(err = GetCStringFromHandle(p->IPStr, host, MAX_URL_LEN))
		goto done;
	
	memcpy(bufferInfo->hostIP, host, sizeof(char) * strlen(host));
	
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
	if (sockNum < 0) 
		goto done;
	
	FD_ZERO(&tempset);
	FD_SET(sockNum, &tempset);
	
	/* Connect to server */
#ifdef _MACINTOSH_
	fdflags = fcntl(sockNum, F_GETFL, 0);
	fdflags |= O_NONBLOCK;
	fcntl(sockNum, F_SETFL, fdflags);
	/*if you don't ignore SIGPIPE then IGOR could crash if the other side exits prematurely.
	 this is for the case that the connect() operation works, but when you try to send()
	 to it there is nothing connected (or the other end has exited.
	 in this case SIGPIPE is issued and terminates the thread.  We don't want that, we'll
	 just rely on errno = EPIPE.*/
	setsockopt(sockNum, SOL_SOCKET, SO_NOSIGPIPE, (void *)&ignoreSIGPIPE, sizeof(int));
#endif
#ifdef _WINDOWS_
	fdflags = 1;
	if(err2 = ioctlsocket(sockNum, FIONBIO, &fdflags)){
		XOPNotice("SOCKITerr: IOCTL failed \r");
		goto done;
	}
#endif
	
	rc = connect(sockNum, (struct sockaddr *)&sa, sizeof(sa));
	res = select(sockNum+1, 0, &tempset, 0, &timeout);

	if(res > 0 && FD_ISSET(sockNum, &tempset)){
	//connection succeeded
	} else {
		close(sockNum);
		sockNum = -1;
		err2 = 1;
		goto done;
	}

	//reset to blocking
#ifdef _MACINTOSH_
	fdflags = fcntl(sockNum, F_GETFL, 0);
	fdflags &= ~(O_NONBLOCK);
	res = fcntl(sockNum, F_SETFL, fdflags);
#endif
	
#ifdef _WINDOWS_
	fdflags = 0;
	if(err2 = ioctlsocket(sockNum, FIONBIO, &fdflags)){
		XOPNotice("SOCKITerr: IOCTL failed \r");
		goto done;
	}
#endif
	
	//socket succeeded in connecting, add to the map containing all the open connections, connect a processor
	if(sockNum > 0){
		if(err = pinstance->addWorker(sockNum, *bufferInfo)){
			err = SOCKET_ALLOC_FAILED;
			goto done;
		}
	}
	
done:
	FD_ZERO(&tempset);
	if(!err && sockNum > 0 && !err2){
		p->retval = sockNum;
	} else {
		p->retval = -1;
		delete bufferInfo;
	}
	
	pthread_mutex_unlock( &readThreadMutex );
	
	return err;
}

