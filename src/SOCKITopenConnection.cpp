#include "CurrentConnections.h"
#include "SOCKITopenconnection.h"
#include "errno.h"

int
RegisterSOCKITopenconnection(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the SOCKITopenconnectionRuntimeParams structure as well.
	cmdTemplate = "SOCKITopenconnection/NOID/TIME=number/LOG=name/Q[=number]/Tok=string/proc=name varname:ID,string:IP,number:PORT,wave:BUF";
	runtimeNumVarList = "V_flag";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITopenconnectionRuntimeParams), (void*)ExecuteSOCKITopenconnection, 0);
}

static int
ExecuteSOCKITopenconnection(SOCKITopenconnectionRuntimeParamsPtr p)
{
	int err = 0, err2 = 0;
	
//	extern CurrentConnections* pinstance;
//	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
	
#ifdef WINIGOR
	extern WSADATA globalWsaData;
#endif
	
	int rc;
    SOCKET sockNum = -1L;
	int res = 0;
	char port[PORTLEN+1];
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
	char unixpath[MAX_PATH_LEN + 1];	
	char *isMAC = NULL;
	
	memset(fnamepath, 0, sizeof(char) * (MAX_PATH_LEN + 1));
	memset(nativepath, 0, sizeof(char) * (MAX_PATH_LEN + 1));
	memset(unixpath, 0, sizeof(char) * (MAX_PATH_LEN + 1));
	memset(report, 0, sizeof(char) * (MAX_MSG_LEN + 1));
	
	if(p->TIMEFlagEncountered){
		timeout.tv_sec = (long) floor(p->TIMEFlagNumber);
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
	bufferInfo->logFile = NULL;
	
	if(p->IDEncountered){
		if(err = VarNameToDataType(p->IDVarName, &dataType)) 
			goto done;
		if(dataType != NT_FP64){
			err = EXPECTED_NUM_VAR_OR_NVAR;
			goto done;
		}
	}
	
	if(p->LOGFlagEncountered && p->LOGFlagParamsSet){
		if(err = GetPathInfo2(p->LOGFlagName, fnamepath))
		   goto done;
		//get native filesystem filepath
		if (err = GetNativePath(fnamepath, nativepath))
			goto done;
		
		
#ifdef MACIGOR
		//see if its a MAC path by seeing if there is the Mac delimiter : in there
		if((isMAC = strstr(nativepath, ":")) && (err = HFSToPosixPath(nativepath, unixpath, 0)))
			goto done;
		if(isMAC){
			strcpy(nativepath, unixpath);
			//and append a trailing /
			strncat(nativepath, "/", MAX_PATH_LEN - 1);
		}
#endif
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
		BCInt tocopy;
		tocopy = GetHandleSize(p->TOKFlagStrH) > 30 ? 30 : GetHandleSize(p->TOKFlagStrH);
		memcpy(bufferInfo->tokenizer, *(p->TOKFlagStrH), tocopy);
		//we don't use strlen because we're interested in 0x00
		//that would normally terminate a string.
		bufferInfo->sztokenizer = (int) GetHandleSize(p->TOKFlagStrH);		
	}
	
	if (p->BUFEncountered) {
		// Parameter: p->BUFWaveH (test for NULL handle before using)
		if(!p->BUFWaveH || (WaveType(p->BUFWaveH) != TEXT_WAVE_TYPE)){
			err = EXPECTED_TEXT_WAVE;
			goto done;
		}
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
	snprintf(port, PORTLEN, "%d", (long)p->PORTNumber);
	snprintf(bufferInfo->port, PORTLEN, "%d", (long)p->PORTNumber);
	
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
#ifdef MACIGOR
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
	
#ifdef WINIGOR
	fdflags = 1;
	if(err2 = ioctlsocket(sockNum, FIONBIO, &fdflags)){
		XOPNotice("SOCKITerr: IOCTL failed \r");
		goto done;
	}
#endif

	rc = connect(sockNum, servinfo->ai_addr, servinfo->ai_addrlen);
	//rc returns -1, with EINPROGRESS, because we set the socket to blocking.  Therefore you 
	//have to test if the socket is writable with select
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
#ifdef MACIGOR
	fdflags = fcntl(sockNum, F_GETFL, 0);
	fdflags &= ~(O_NONBLOCK);
	res = fcntl(sockNum, F_SETFL, fdflags);
#endif
	
#ifdef WINIGOR
	fdflags = 0;
	if(err2 = ioctlsocket(sockNum, FIONBIO, &fdflags)){
		XOPNotice("SOCKITerr: IOCTL failed \r");
		goto done;
	}
#endif
	/*
	 set the timeout for the blocking socket.
	NOTE: normally the readerthread and all other reads use select to figure out whether there is something
	 to be read.  However, in my experience select has failed sometimes, i.e. select indicates something
	 is available to be read, but there is nothing there.  Therefore, the recv fails.
	 */

//#ifdef _MACINTOSH_
//	memset(&timeout, 0, sizeof(timeout));
//	timeout.tv_usec = 1000;
//	if(err2 = setsockopt(sockNum, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)))
//	   goto done;
//	if(err2 = setsockopt(sockNum, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(int)))
//		goto done;
//#endif
//#ifdef _WINDOWS_
//	DWORD socktimeout = 1000;
//	if(err2 = setsockopt(sockNum, SOL_SOCKET, SO_RCVTIMEO, (char*) &socktimeout, sizeof(socktimeout)))
//	   goto done;
//	if(err2 = setsockopt(sockNum, SOL_SOCKET, SO_RCVBUF, (char*) &bufsize, sizeof(int)))
//		goto done;
//#endif

	//socket succeeded in connecting, add to the map containing all the open connections, connect a processor
	if(sockNum > 0){
		if(strlen(nativepath) > 0)
			memcpy(bufferInfo->logFilePath, nativepath, sizeof(char) * strlen(nativepath));

		bufferInfo->sockNum = sockNum;
		
		if(err = pinstance->addWorker(sockNum, *bufferInfo, p->BUFWaveH))
			goto done;
		
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
	
//	extern CurrentConnections* pinstance;
//	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
	
	p->retval=-1;
	
#ifdef WINIGOR
	extern WSADATA globalWsaData;
#endif
	
	int rc;
    SOCKET sockNum = -1;
	int res = 0;
	char port[7];
	unsigned long fdflags;
	int ignoreSIGPIPE = 1;
	
	char host[MAX_URL_LEN+1];
		struct addrinfo hints, *servinfo = NULL;

	fd_set tempset;	
	struct timeval timeout;
	
	waveBufferInfo *bufferInfo = new waveBufferInfo();
		
	timeout.tv_sec = (long) floor(p->timeout);
	timeout.tv_usec =  (long)((p->timeout-(double)floor(p->timeout))*1000000);
	
	bufferInfo->NOIDLES = true;
	bufferInfo->toPrint = false;
	bufferInfo->logFile = NULL;
			
	if(!p->IPStr){
		err = OH_EXPECTED_STRING;
		goto done;
	}
	if(err = GetCStringFromHandle(p->IPStr, host, MAX_URL_LEN))
		goto done;
	
	memcpy(bufferInfo->hostIP, host, sizeof(char) * strlen(host));
	
	/* Address resolution */	
	memset(&hints, 0, sizeof(hints));
	memset(port, 0, sizeof(char) * 7);
	
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	snprintf(port, 6, "%d", (long)p->portNumber);
	snprintf(bufferInfo->port, PORTLEN, "%d", (long)p->portNumber);

	if(getaddrinfo(host, port, &hints, &servinfo)){
		err = BAD_HOST_RESOLV;
		goto done;
	}
	
	/* allocate a socket */
	sockNum = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (sockNum < 0) 
		goto done;
	
	FD_ZERO(&tempset);
	FD_SET(sockNum, &tempset);
	
	/* Connect to server */
#ifdef MACIGOR
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
	
#ifdef WINIGOR
	fdflags = 1;
	if(err2 = ioctlsocket(sockNum, FIONBIO, &fdflags))
		goto done;
#endif
	
	rc = connect(sockNum, servinfo->ai_addr, servinfo->ai_addrlen);
	//rc returns -1, with EINPROGRESS, because we set the socket to blocking.  Therefore you 
	//have to test if the socket is writable with select
	res = select(sockNum + 1, 0, &tempset,0, &timeout);
	
	if(res > 0 && FD_ISSET(sockNum, &tempset)){
	} else {
		close(sockNum);
		sockNum = -1;
		err2 = 1;
		goto done;
	}
	
	//reset to blocking
#ifdef MACIGOR
	fdflags = fcntl(sockNum, F_GETFL, 0);
	fdflags &= ~(O_NONBLOCK);
	res = fcntl(sockNum, F_SETFL, fdflags);
#endif
	
#ifdef WINIGOR
	fdflags = 0;
	if(err2 = ioctlsocket(sockNum, FIONBIO, &fdflags)){
		XOPNotice("SOCKITerr: IOCTL failed \r");
		goto done;
	}
#endif
	
	/*
	 set the timeout for the blocking socket.
	 NOTE: normally the readerthread and all other reads use select to figure out whether there is something
	 to be read.  However, in my experience select has failed sometimes, i.e. select indicates something
	 is available to be read, but there is nothing there.  Therefore, the recv fails.
	 */
	
//#ifdef _MACINTOSH_
//	memset(&timeout, 0, sizeof(timeout));
//	timeout.tv_usec = 1000;
//	if(err2 = setsockopt(sockNum, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)))
//	   goto done;
//	if(err2 = setsockopt(sockNum, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(int)))
//		goto done;
//#endif
//#ifdef _WINDOWS_
//	DWORD socktimeout = 1000;
//	if(err2 = setsockopt(sockNum, SOL_SOCKET, SO_RCVTIMEO, (char*) &socktimeout, sizeof(socktimeout)))
//	   goto done;
//	if(err2 = setsockopt(sockNum, SOL_SOCKET, SO_RCVBUF, (char*) &bufsize, sizeof(int)))
//		goto done;
//#endif

	//socket succeeded in connecting, add to the map containing all the open connections, connect a processor
	if(sockNum > 0){
		bufferInfo->sockNum = sockNum;
		if(err = pinstance->addWorker(sockNum, *bufferInfo, NULL)){
			err = SOCKET_ALLOC_FAILED;
			goto done;
		}
	}
	
done:
	if(servinfo)
		freeaddrinfo(servinfo);
	
	FD_ZERO(&tempset);
	if(!err && sockNum > 0 && !err2){
		p->retval = sockNum;
	} else {
		p->retval = -1;
	}
	if(bufferInfo)
		delete bufferInfo;
	
	pthread_mutex_unlock( &readThreadMutex );
	
	return err;
}

