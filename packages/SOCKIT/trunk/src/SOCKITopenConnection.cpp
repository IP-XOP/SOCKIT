#include "SOCKIT.h"

int 
SOCKITopenConnection(SOCKITopenConnectionStruct *p){
	int err = 0;
	
	extern currentConnections openConnections;
	#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
	#endif
	
	int rc;
    SOCKET sockNum = -1;
    long port; 
	int res = 0;
	
	char host[MAX_URL_LEN+1];
	char report[MAX_MSG_LEN+1];
	char processor[MAX_OBJ_NAME+1];
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

    /* From XOP programming guide p337 */
    /*GetCStringFromHandle(p->host, host, sizeof(host)-1);*/
    /* Address resolution */
	
	if(!p->tokenizer){
		err = OH_EXPECTED_STRING;
		goto done;
	}
	
	if(err = GetCStringFromHandle(p->processor,processor,sizeof(processor)))
		goto done;
	
	if(!p->bufferWave || (WaveType(p->bufferWave) != TEXT_WAVE_TYPE)){
		err = EXPECTED_TEXT_WAVE;
		goto done;
	}
	
	if(!p->IPaddress){
		err = OH_EXPECTED_STRING;
		goto done;
	} else {
		if(err = GetCStringFromHandle(p->IPaddress, host, sizeof(host)))
			goto done;
/**
#ifdef _WINDOWS_
    if (isalpha(host[0])) {        
         hen = gethostbyname(host);
    } else {
        sa.s_addr = inet_addr(host);
        if (sa.s_addr == INADDR_NONE) {
           err = BAD_HOST_RESOLV;
           goto done;
        } else
            hen = gethostbyaddr((char *) &sa, 4, AF_INET);
    }
#endif
**/
		hen = gethostbyname(host);
		if (!hen){
			err = BAD_HOST_RESOLV;
			goto done;
		}
	}
	
	if(!p->port){
		err = OH_EXPECTED_NUMBER;
		goto done;
	} else {
		port = p->port;
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
	//socket succeeded in connecting, attach a buffer wave, connect a processor
	if(sockNum>0){
		bufferInfoStruct.bufferWave = p->bufferWave;
		bufferInfoStruct.toPrint = p->toPrint;
		if(err = GetCStringFromHandle(p->tokenizer,bufferInfoStruct.tokenizer,10))
			goto done;
			
		openConnections.bufferWaves[sockNum] = bufferInfoStruct;
		
		if(err = registerProcessor(sockNum, processor)){
			err = PROCESSOR_NOT_AVAILABLE;
			goto done;
		}
	}
	
		
done:
	if (p->IPaddress)
		DisposeHandle(p->IPaddress);			/* we need to get rid of input parameters */
	if(p->tokenizer)
		DisposeHandle(p->tokenizer);
    if(p->processor)
		DisposeHandle(p->processor);
		
	FD_ZERO(&tempset);
    p->retval = sockNum;


return err;
}