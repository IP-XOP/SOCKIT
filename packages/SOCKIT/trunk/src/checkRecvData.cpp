// Process events of other sockets... 
#include "SOCKIT.h"
#include<time.h>
#include<string>

using namespace std;


/**
•make/t poo
•variable sock = sockitopenconnection("www.wavemetrics.com",80,poo,"","",1)
•sockitsendmsg(sock,"GET / \r\n")
**/

int GetHourMinuteSecond(long* h, long* m, long* s)
{
#ifdef _MACINTOSH_
	DateTimeRec date;

	GetTime(&date);
	*h = date.hour;
	*m = date.minute;
	*s = date.second;
#endif
#ifdef _WINDOWS_
	SYSTEMTIME st;
	
	GetLocalTime(&st);
	*h = st.wHour;
	*m = st.wMinute;
	*s = st.wSecond;
#endif
	return 0;
}

int outputBufferDataToWave(SOCKET sockNum, waveHndl wavH, const char *writebuffer, const char *tokenizer){
	int err = 0;
	
	extern currentConnections openConnections;
	
	long numDimensions = 2; 
	long dimensionSizes[MAX_DIMENSIONS+1]; 
	long indices[MAX_DIMENSIONS];
	
	Handle textH = NULL;
	vector<string> tokens;
	SOCKET ii = 0;
	
	DataFolderHandle dfH;
	char pathName[MAXCMDLEN+1];
	char waveName[MAX_WAVE_NAME+1];
	char cmd[MAXCMDLEN+1];
	char pointsToDeleteStr[MAXCMDLEN+1];
	long pointsToDelete = 0;

	long hour,min,sec;
	char timebuf[10];
	
	SOCKITcallProcessorStruct callProcessor;
	double result;
	
	textH = NewHandle(10); 
	if (textH == NULL) {
		err = NOMEM; 
		goto done;
	}
	
	Tokenize(writebuffer, tokens, tokenizer);
	
	for(ii=0 ; ii< tokens.size(); ii++){
		
		if(err = PutCStringInHandle(tokens.at(ii).c_str(),textH))
			goto done;
		
		// Clear all dimensions sizes to avoid undefined values. 
		MemClear(dimensionSizes, sizeof(dimensionSizes)); 
		MemClear(indices, sizeof(indices)); 
		MemClear(cmd, sizeof(cmd)); 
		
		if (err = MDGetWaveDimensions(wavH, &numDimensions, dimensionSizes)) 
			goto done; 
		
		dimensionSizes[0] +=1;
		dimensionSizes[1] = 2;    // 2 columns 
		dimensionSizes[2] = 0;    // 0 layers 
		
		if(err = MDChangeWave(wavH,-1,dimensionSizes))
			goto done;
		
		indices[0] = dimensionSizes[0]-1;
		indices[1] = 0;
		
		if(err = MDSetTextWavePointValue(wavH,indices,textH))
			goto done;
		
		GetHourMinuteSecond(&hour,&min,&sec);
		snprintf(timebuf,sizeof(timebuf),"%02ld:%02ld:%02ld",hour,min,sec);
		if(err = PutCStringInHandle(timebuf,textH))
			goto done;
		indices[1] = 1;
		
		if(err = MDSetTextWavePointValue(wavH,indices,textH))
			goto done;
		
		if(dimensionSizes[0] > BUFFER_WAVE_LEN){
			pointsToDelete = dimensionSizes[0] - BUFFER_WAVE_LEN;
			indices[0] -= pointsToDelete;
			
			if(err = GetWavesDataFolder(wavH,&dfH))
				goto done;
			if(err = GetDataFolderNameOrPath(dfH,1,pathName))
				goto done;
			WaveName(wavH,waveName);
			strlcat(pathName,waveName,sizeof(pathName));
			strlcpy(cmd,"Deletepoints 0,",sizeof(cmd));
			snprintf(pointsToDeleteStr,sizeof(pointsToDeleteStr),"%d,",pointsToDelete);
			strlcat(cmd,pointsToDeleteStr,sizeof(cmd));
			strlcat(cmd,pathName,sizeof(cmd));
			if(err = XOPCommand2(cmd,0,0))
				goto done;
		}
		
		//call a processor for each buffer entry to see if there's anything it has to do with it.
		if(!checkProcessor(openConnections.bufferWaves[sockNum].processor,  openConnections.bufferWaves[sockNum].processorfip)){
			callProcessor.entryRow = indices[0];
			callProcessor.bufferWave = wavH;
			if(err = CallFunction(openConnections.bufferWaves[sockNum].processorfip,&callProcessor,&result))
				goto done;
		}
		
		//WaveHandleModified(wavH);
	}
done:
		if(textH!=NULL)
			DisposeHandle(textH);
	return err;
}

int checkRecvData(){
	int err = 0;
	extern currentConnections openConnections;
	
	#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
	#endif
	
	SOCKET maxSockNum = openConnections.maxSockNumber;
	SOCKET ii;
	
	char buf[BUFLEN+1];
	
	struct MemoryStruct chunk;
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
    chunk.size = 0;    /* no data at this point */
	
	int rc = 0, res = 0;
	int iters =0;
	long charsread = 0;
	char *ending = "\0";
	
	char report[MAX_MSG_LEN+1];

	char* output = NULL;
	fd_set tempset;
	FD_ZERO(&tempset);
	
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	memcpy(&tempset, &openConnections.readSet, sizeof(openConnections.readSet)); 
	res = select(maxSockNum+1,&tempset,0,0,&timeout);
	if(res ==0)
		goto done;

	for (ii=0; ii<maxSockNum+1; ii++) { 
		iters = 0;
		charsread = 0;

		if (FD_ISSET(ii, &tempset)) {
			while(FD_ISSET(ii, &tempset)){
				iters += 1;

#ifdef _MACINTOSH_
				rc = recv(ii, buf, BUFLEN,0);
#endif
#ifdef _WINDOWS_
				rc = recv(ii, buf, BUFLEN,0);
#endif
				charsread += rc;
				
				if (rc < 0) { 
					snprintf(report,sizeof(report),"SOCKIT err: problem reading socket descriptor %d, disconnection???\r", ii );
					XOPNotice(report);
					// Closed connection or error 
					SOCKITcloseWorker(ii);
					break;
				} else if(rc > 0){
					WriteMemoryCallback(buf, sizeof(char), rc, &chunk);
					if(chunk.memory == NULL){
					   err = NOMEM;
					   goto done;
				   }
				} else if (rc == 0)
					break;
				timeout.tv_sec = 3;

				memcpy(&tempset, &openConnections.readSet, sizeof(openConnections.readSet)); 
				res = select(maxSockNum+1,&tempset,0,0,&timeout);
			}
			if(charsread>0){
				WriteMemoryCallback(ending, sizeof(char), strlen(ending), &chunk);
					if(chunk.memory == NULL){
					   err = NOMEM;
					   goto done;
				   }
				if(openConnections.bufferWaves[ii].toPrint == true){
					snprintf(report,sizeof(report),"SOCKITmsg: Socket %d says: \r", ii);
					XOPNotice(report);
					output = NtoCR(chunk.memory, "\n","\r");
					XOPNotice(output);
					if(output){
						free(output);
						output = NULL;
					}
				}
				if(err = outputBufferDataToWave(ii, openConnections.bufferWaves[ii].bufferWave, chunk.memory, openConnections.bufferWaves[ii].tokenizer))
					goto done;
			}

			timeout.tv_sec = 0.;
		}
		if(chunk.memory){
			free(chunk.memory);
			chunk.memory = NULL;
			chunk.size = 0;	
		}
	}
	
done:
	FD_ZERO(&tempset);
	if(output!= NULL)
		free(output);
	if(chunk.memory)
		free(chunk.memory);
	
	return err;
}







