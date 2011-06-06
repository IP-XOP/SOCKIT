/*
 *  Connections.cpp
 *  iPeek
 *
 *  Created by andrew on 17/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#include "CurrentConnections.h"

#include <map>
#include <algorithm>
#include <vector>
#include <string.h>
#include <ctime>

using namespace std;


CurrentConnections* pinstance=0;
pthread_t *readThread=0;
pthread_mutex_t readThreadMutex = PTHREAD_MUTEX_INITIALIZER;


/*
	roundDouble returns a rounded value for val
 */
double
roundDouble(double val){
	double retval;
	if(val>0){
		if(val-floor(val) < 0.5){
			retval = floor(val);
		} else {
			retval = ceil(val);
		}
	} else {
		if(val-floor(val) <= 0.5){
			retval = floor(val);
		} else {
			retval = ceil(val);
		}
	}
	return retval;
}

long doubleToLong(double val){
	long retval;
	DoubleToLong(&val, &retval,1);
	
	return retval;
}

void *readerThread(void *){
    extern CurrentConnections* pinstance;
	extern pthread_mutex_t readThreadMutex;
	
#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif
	
	SOCKET maxSockNum;
	SOCKET ii;
	
	int rc = 0, res = 0, iters=0;
	long charsread = 0;		
	char buf[BUFLEN];
	
	fd_set tempset;
	struct timeval sleeper;
	
    for(;;){
		if(pthread_mutex_trylock( &readThreadMutex ))
			continue;
			
		FD_ZERO(&tempset);
		
		if(pinstance->quitReadThreadStatus()){
			pthread_mutex_unlock(&readThreadMutex);
//#ifdef _WINDOWS_
//			pthread_win32_thread_detach_np ();
//#endif
			pthread_exit(NULL);
		}
		
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		maxSockNum = pinstance->getMaxSockNumber();
		memcpy(&tempset, pinstance->getReadSet(), sizeof(fd_set));
		
		res = select(maxSockNum+1, &tempset, 0, 0, &timeout);
		if(res > 0){
			for (ii=0; ii<maxSockNum + 1; ii++) { 
				if (FD_ISSET(ii, &tempset) && pinstance->getWaveBufferInfo(ii)->toClose == false) {
					iters = 0;
					charsread = 0;
					rc = 0;
					memset(buf, 0, BUFLEN * sizeof(char));
					do{
						memset(buf, 0, BUFLEN * sizeof(char));
						iters += 1;
						//read the characters from the socket
						rc = recv(ii, buf, BUFLEN, 0);
						charsread += rc;

						if (rc <= 0 && iters == 1) {
							pinstance->getWaveBufferInfo(ii)->toClose = true;
						} else if(rc > 0){
							if(pinstance->getWaveBufferInfo(ii)->readBuffer.append(buf, sizeof(char), rc) == -1)
								pinstance->getWaveBufferInfo(ii)->readBuffer.reset();
						}
					}while (rc == BUFLEN);					
				}
			}
			
		}
		pthread_mutex_unlock( &readThreadMutex );

#ifdef _WINDOWS_
		Sleep(50);
#endif
#ifdef _MACINTOSH_
		sleeper.tv_sec = 0;
		sleeper.tv_usec = 10000;
		select(0,0,0,0,&sleeper);
#endif
	}
}

int GetTheTime(long *year, long *month, long *day, long *hour, long *minute, long *second){
	time_t rawtime;
	tm *theTime ;
	
	time(&rawtime);
	theTime = localtime(&rawtime);
	*year = -100 + theTime->tm_year;
	*month = 1+theTime->tm_mon;
	*day = theTime->tm_mday;
	*hour = theTime->tm_hour;
	*minute = theTime->tm_min;
	*second = theTime->tm_sec; 
	return 0;
}

void GetTimeStamp(char timestamp[101]){
	long year, month, day, hour, minute, second;
	GetTheTime(&year, &month, &day, &hour, &minute, &second);
	snprintf(timestamp, 100 * sizeof(char), "%d-%d-%d_T%d:%d:%d",year, month, day, hour, minute, second );
}

void CurrentConnections::Instance(){
	extern CurrentConnections* pinstance;
	
	if(pinstance == 0)
		pinstance = new CurrentConnections(); // create sole instance
}

CurrentConnections::CurrentConnections(){
	FD_ZERO(&readSet);
	maxSockNumber = 0;
	quitReadThreadFlag = false;
	bufferWaves.clear();
	totalSocketsOpened = 0;
};

CurrentConnections::~CurrentConnections(){
};


void CurrentConnections::resetCurrentConnections(){
	SOCKET ii;
	for (ii=0; ii< maxSockNumber+1 ; ii+=1){
		if (FD_ISSET(ii, &(readSet))) { 
			FD_CLR(ii, &(readSet)); 
			close(ii);
		} 
	}
	FD_ZERO((&(pinstance->readSet)));
	maxSockNumber = 0;
	bufferWaves.clear();
}

SOCKET CurrentConnections::getMaxSockNumber(){
	return maxSockNumber;
}

fd_set* CurrentConnections::getReadSet(){
	return &readSet;
};

void CurrentConnections::quitReadThread(){
	quitReadThreadFlag = true;
}

bool CurrentConnections::quitReadThreadStatus(){
	return quitReadThreadFlag;
}

void CurrentConnections::resetMaxSocketNumber(){
	
	SOCKET ii=0;
	SOCKET maxSoFar=0;
	
	for (ii=0; ii< maxSockNumber+1 ; ii+=1){
		if (FD_ISSET(ii, &(readSet)))
			maxSoFar = ii;
	}
	maxSockNumber = maxSoFar;
}

waveBufferInfo* CurrentConnections::getWaveBufferInfo(SOCKET sockNum){
	return &(bufferWaves[sockNum]);
};


int CurrentConnections::closeWorker(SOCKET sockNum){
	int err = 0;

	
	char report[MAX_MSG_LEN+1];
	char timestamp[101];
	waveBufferInfo *wbi;

	GetTimeStamp(timestamp);
	/* Disconnect from server */
	if (FD_ISSET(sockNum, &(readSet))) { 
		FD_CLR(sockNum, &(readSet)); 
		close(sockNum);
		resetMaxSocketNumber();
	}
	wbi = getWaveBufferInfo(sockNum);
	
	if(wbi->logFile){
		snprintf(report,
				 sizeof(char) * MAX_MSG_LEN,
				 "%s\tSOCKCLOSE:\t%d\t", timestamp, sockNum);
		
		fwrite(report, sizeof(char), strlen(report), wbi->logFile);
		fwrite("\r\n", sizeof(char), 2,  wbi->logFile);	
	}
		
	bufferWaves.erase(sockNum);
	
	return err;
}

int CurrentConnections::addWorker(SOCKET sockNum, waveBufferInfo &bufferInfoStruct){
	int err = 0;
	waveBufferInfo *wbi;
	char timestamp[101];
	char report[MAX_MSG_LEN+1];
	
	bufferWaves.insert(make_pair(sockNum, bufferInfoStruct));

	FD_SET(sockNum,&(readSet));
	if(sockNum > maxSockNumber){
		maxSockNumber = sockNum;
	}
	wbi = getWaveBufferInfo(sockNum);
	
	totalSocketsOpened += 1;
	
	if(strlen(wbi->logFileName)){
		XOPOpenFile(wbi->logFileName, 1, &wbi->logFile);
		fseek(wbi->logFile, 0, 1);
		int pos = ftell(wbi->logFile);
		
		if(!wbi->logFile)
			XOPNotice("SOCKIT err: couldn't create logfile)\r");
		else {
			GetTimeStamp(timestamp);
			snprintf(report,
					 sizeof(char) * MAX_MSG_LEN,
					 "%s\tSOCKOPEN:\t%d\tIP\t%s\r\n", timestamp, sockNum, wbi->hostIP);
			
			fwrite(report, sizeof(char), strlen(report), wbi->logFile);
		}
	}
	
	done:
	return err;
};


int CurrentConnections::checkIfWaveInUseAsBuf(waveHndl wav){
	int inUse = 0;
	int ii;
		
	for (ii=0; ii< maxSockNumber+1 ; ii+=1){
		if (FD_ISSET(ii, &(readSet))) { 
			if(bufferWaves[ii].bufferWave == wav)
				return 1;
		} 
	}
	
	return inUse;
}


int CurrentConnections::registerProcessor(SOCKET sockNum, const char *processor){
	int err=0;
	
	FunctionInfo fi;
    
	memset(bufferWaves[sockNum].processor,0, sizeof(bufferWaves[sockNum].processor));
	strlcpy(bufferWaves[sockNum].processor, processor, sizeof(bufferWaves[sockNum].processor));
	
	if(strlen(processor)==0)
		goto done;
	
	if(err = GetFunctionInfo(processor, &fi))
		err = PROCESSOR_NOT_AVAILABLE;
	
	if(err = checkProcessor(sockNum, &fi))
		err = PROCESSOR_NOT_AVAILABLE;
	
done:
	return err;
}

int CurrentConnections::isSockitOpen(double query,SOCKET *sockNum){
	int retval=0;
	*sockNum = (SOCKET)-1;
	double roundedVal;
	long intVal;
	map<SOCKET, waveBufferInfo>::iterator iter;
	
	if(IsNaN64(&query))
		return 0;
	if(IsINF64(&query))
		return 0;
	if(query<=0)
		return 0;
	
	roundedVal = roundDouble(query);
	intVal = doubleToLong(roundedVal);
	if(intVal<0)
		return 0;
	
	//see if that socket is still contained in the map.
	iter = bufferWaves.find((SOCKET)intVal);
	if(	iter != bufferWaves.end() && FD_ISSET(intVal, pinstance->getReadSet())){
		*sockNum = (SOCKET)intVal;
		retval = 1;
	 } else
		retval = 0;
	
	return retval;
}

int CurrentConnections::checkProcessor(SOCKET sockNum, FunctionInfo *fip){
	int err=0;
	
	int requiredParameterTypes[2];
	
	requiredParameterTypes[0] = TEXT_WAVE_TYPE;
	requiredParameterTypes[1] = NT_FP64;
	int badParam = 0;
	
	const char* processor = bufferWaves[sockNum].processor;
	
	if(strlen(processor)==0){
		err = 0;
		goto done;
	}
	
	if(err = GetFunctionInfo(processor,fip))
		err = PROCESSOR_NOT_AVAILABLE;
	
	if(err = CheckFunctionForm(fip, 2, requiredParameterTypes, &badParam,NT_FP64))
		err = PROCESSOR_NOT_AVAILABLE;
	
done:
	return err;
}

int CurrentConnections::checkRecvData(){
	int err = 0;
	
	char report[MAX_MSG_LEN+1];
	
	map<SOCKET,waveBufferInfo>::iterator iter;
    for( iter = bufferWaves.begin(); iter != bufferWaves.end(); ++iter ) {
		if(!bufferWaves[iter->first].NOIDLES && bufferWaves[iter->first].readBuffer.getMemSize()>0 && bufferWaves[iter->first].readBuffer.getData()){

			if(err = outputBufferDataToWave(iter->first, bufferWaves[iter->first].readBuffer.getData(),  bufferWaves[iter->first].readBuffer.getMemSize(), true))
				goto done;
			if(bufferWaves[iter->first].toPrint == true){
				memset(report, 0, sizeof(report));
				snprintf(report,sizeof(report),"SOCKITmsg: Socket %d says: \r", iter->first);
				XOPNotice(report);
				
				string output;
				output = string((const char*)bufferWaves[iter->first].readBuffer.getData(), bufferWaves[iter->first].readBuffer.getMemSize());
				find_and_replace(output,"\n","\r");
				XOPNotice(output.c_str());
				XOPNotice("\r");
			}
			bufferWaves[iter->first].readBuffer.reset();
			if(bufferWaves[iter->first].toClose){
				memset(report, 0, sizeof(report));
				snprintf(report,sizeof(report),"SOCKITmsg: closing socket %d\r", iter->first);
				pinstance->closeWorker(iter->first);
			}
		}
	}
	
done:	
	return err;
}

int CurrentConnections::outputBufferDataToWave(SOCKET sockNum, const unsigned char *writebuffer, size_t szwritebuffer, bool useProcessor){
	int err = 0;
	
	long numDimensions = 2; 
	long dimensionSizes[MAX_DIMENSIONS+1]; 
	long indices[MAX_DIMENSIONS];
	
	Handle textH = NULL;
	vector<string> tokens;
	SOCKET ii = 0;
	
	DataFolderHandle dfH;
	char pathName[MAXCMDLEN + 1];
	char waveName[MAX_WAVE_NAME + 1];
	char cmd[MAXCMDLEN + 1];
	char report[MAX_MSG_LEN+1];
	char pointsToDeleteStr[MAXCMDLEN + 1];
	long pointsToDelete = 0;
	
	char timestamp[101];
	
	SOCKITcallProcessorStruct callProcessor;
    FunctionInfo fi;
	waveBufferInfo *wbi = NULL;
    waveHndl wav = NULL;
	
	//do you want to debug?
	bool DBUG = false;	
	double result;
	
	wbi = getWaveBufferInfo(sockNum);
	
	if(!wbi){
		err = NO_WAVE_BUFFER_INFO;
		goto done;
	}
	
	DBUG = wbi->DBUG;
	wav = wbi->bufferWave;
	
	if(!wav){
		err = NO_WAVE_BUFFER_INFO;
		goto done;
	}
		
	textH = NewHandle(10); 
	if (textH == NULL) {
		err = NOMEM; 
		goto done;
	}
	
	Tokenize(writebuffer, szwritebuffer, tokens, bufferWaves[sockNum].tokenizer,bufferWaves[sockNum].sztokenizer);
	
	for(ii=0 ; ii< tokens.size(); ii++){
		
		if(err = PtrToHand((Ptr)tokens.at(ii).data(), &textH,tokens.at(ii).length()))
			goto done;
		
		// Clear all dimensions sizes to avoid undefined values. 
		MemClear(dimensionSizes, sizeof(dimensionSizes)); 
		MemClear(indices, sizeof(indices)); 
		MemClear(cmd, sizeof(cmd)); 
		
		if (err = MDGetWaveDimensions(wav, &numDimensions, dimensionSizes)) 
			goto done; 
		
		dimensionSizes[0] +=1;
		if(!DBUG)
			dimensionSizes[1] = 2;    // 2 columns 
		else
			dimensionSizes[1] = 3;
		dimensionSizes[2] = 0;    // 0 layers 
		
		if(err = MDChangeWave(wav,-1,dimensionSizes))
			goto done;
		
		indices[0] = dimensionSizes[0]-1;
		indices[1] = 0;
		
		if(err = MDSetTextWavePointValue(wav,indices,textH))
			goto done;
		
		GetTimeStamp(timestamp);
		
		if(err = PutCStringInHandle(timestamp, textH))
			goto done;
		indices[1] = 1;
		
		if(err = MDSetTextWavePointValue(wav,indices,textH))
			goto done;
		
		//to Debug put the socket number in the 3rd column
		if(DBUG){
			snprintf(timestamp, sizeof(char) * 10, "%d", sockNum);
			if(err = PutCStringInHandle(timestamp, textH))
				goto done;
			indices[1] = 2;
			if(err = MDSetTextWavePointValue(wav,indices,textH))
				goto done;
		}
		
		if(dimensionSizes[0] > BUFFER_WAVE_LEN){
			pointsToDelete = 300;//dimensionSizes[0] - BUFFER_WAVE_LEN;
			indices[0] -= pointsToDelete;
			
			if(err = GetWavesDataFolder(wav, &dfH))
				goto done;
			if(err = GetDataFolderNameOrPath(dfH, 1, pathName))
				goto done;
			WaveName(wav, waveName);
			strlcat(pathName,waveName,sizeof(pathName));
			strlcpy(cmd,"Deletepoints 0,",sizeof(cmd));
			snprintf(pointsToDeleteStr,sizeof(pointsToDeleteStr),"%d,",pointsToDelete);
			strlcat(cmd,pointsToDeleteStr,sizeof(cmd));
			strlcat(cmd,pathName,sizeof(cmd));
			if(err = XOPSilentCommand(cmd))
				goto done;
		}
		
		//if there is a logfile then append and save
		if(wbi->logFile){
			snprintf(report,
					 sizeof(char) * MAX_MSG_LEN,
					 "%s\tRECV:\t%d\t", timestamp, sockNum);
			int wrote = 0;
			fwrite(report, sizeof(char), strlen(report), wbi->logFile);
			fwrite(tokens.at(ii).c_str(), sizeof(char) , strlen(tokens.at(ii).c_str()), wbi->logFile);
			fwrite("\r\n", sizeof(char), 2,  wbi->logFile);		
		}
		
		WaveHandleModified(wav);
	
		//call a processor for each buffer entry to see if there's anything it has to do with it.
		if(useProcessor==true){
			if(checkProcessor(sockNum,  &fi)){
				XOPNotice("SOCKIT error: processor must be f(textWave,variable)\r");
			} else {
				if(strlen(wbi->processor)==0)
					continue;
				callProcessor.entryRow = indices[0];
				callProcessor.bufferWave = wav;
				if(err = CallFunction(&fi, &callProcessor, &result))
					goto done;
			}
		}
	}
done:
	if(textH!=NULL)
		DisposeHandle(textH);
	
	return err;
}

long CurrentConnections::getTotalSocketsOpened(){
	return totalSocketsOpened;
}

long CurrentConnections::getCurrentSocketsOpened(){
	return (long)bufferWaves.size();
}
