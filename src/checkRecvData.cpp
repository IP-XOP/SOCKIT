// Process events of other sockets... 
#include "SOCKIT.h"
#include<time.h>
#include<string>

using namespace std;

/**
•make/t poo
•variable sock = csockitopenconnection("www.wavemetrics.com",80,poo,"","",1)
•sockitsendmsg(sock,"GET / \r\n")
**/

int GetHourMinuteSecond(long* h, long* m, long* s)
{
	DateTimeRec date;
	
	GetTime(&date);
	*h = date.hour;
	*m = date.minute;
	*s = date.second;
	return 0;
}

int outputBufferDataToWave(long sockNum, waveHndl wavH, const char *buf, const char *tokenizer){
	int err = 0;
	
	extern currentConnections openConnections;
	
	long numDimensions = 2; 
	long dimensionSizes[MAX_DIMENSIONS+1]; 
	long indices[MAX_DIMENSIONS];
	
	Handle textH = NULL;
	vector<string> tokens;
	long ii = 0;
	
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
	
	textH = NewHandle(strlen(buf)-1); 
	if (textH == NULL) {
		err = NOMEM; 
		goto done;
	}
	
	Tokenize(buf, tokens, tokenizer);
	
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
			if(err = XOPCommand2(cmd,1,1))
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
	int maxSockNum = openConnections.maxSockNumber;
	
	char buf[BUFLEN+1];
	char *writebuffer = NULL;
	int iters =0;
	long charsread = 0;
	char ending = '\0';
	
	char report[MAX_MSG_LEN+1];
	int rc = -1, res = 0;
	int ii;
	char* output;
	fd_set tempset;
	FD_ZERO(&tempset);
	
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
	writebuffer = (char*)malloc(1);
	if(writebuffer == NULL){
		err = NOMEM;
		goto done;
	}
	
	memset(writebuffer,0,1);
	
	memcpy(&tempset, &openConnections.readSet, sizeof(openConnections.readSet)); 
	res = select(maxSockNum+1,&tempset,0,0,&timeout);
	
	for (ii=0; ii<maxSockNum+1; ii++) { 
		if (FD_ISSET(ii, &tempset)) {
			while(FD_ISSET(ii, &tempset)){
				iters += 1;
				writebuffer = (char*)realloc(writebuffer,BUFLEN*iters);
				if(writebuffer == NULL){
					err = NOMEM;
					goto done;
				}
				rc = read(ii, buf, BUFLEN); 
				charsread += rc;
				
				if (rc < 0) { 
					snprintf(report,sizeof(report),"SOCKIT err: problem reading socket descriptor %d, disconnection???\r", ii );
					XOPNotice(report);
					// Closed connection or error 
					SOCKITcloseWorker(ii);
					break;
				} else if(rc > 0){
					
				} else if (rc == 0)
					break;
	//			timeout.tv_sec = 10.0;

				memcpy(&tempset, &openConnections.readSet, sizeof(openConnections.readSet)); 
				res = select(maxSockNum+1,&tempset,0,0,&timeout);
				strlcat(writebuffer,buf,BUFLEN);
			}
			if(charsread){
				*(writebuffer+charsread) = ending;
				if(openConnections.bufferWaves[ii].toPrint == true){
					snprintf(report,sizeof(report),"SOCKITmsg: Socket %d says: \r", ii);
					XOPNotice(report);
					output = NtoCR(writebuffer, "\n","\r");
					XOPNotice(output);
				}
				if(err = outputBufferDataToWave(ii, openConnections.bufferWaves[ii].bufferWave, writebuffer, openConnections.bufferWaves[ii].tokenizer))
					goto done;
					
				charsread = 0;

			}
	//		timeout.tv_sec = 0.;
		}
	}
	
done:
		FD_ZERO(&tempset);
	if(output!= NULL)
		free(output);
	if(writebuffer)
		free(writebuffer);
	
	return err;
}







