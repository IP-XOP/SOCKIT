/*
 *  Connections.cpp
 *  SOCKIT
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
#include <sstream>
#include <iterator>
#include "TextWaveAccess.h"

//#include <stdio.h>
//#include <fcntl.h>
//#include <stdlib.h>
//#include <errno.h>




using namespace std;

CurrentConnections *pinstance = NULL;
pthread_t *readThread = NULL;
pthread_mutex_t readThreadMutex = PTHREAD_MUTEX_INITIALIZER;
bool SHOULD_IDLE_SKIP = false;

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

void *readerThread(void *){
//    extern CurrentConnections* pinstance;
//	extern pthread_mutex_t readThreadMutex;
	
#ifdef WINIGOR
	extern WSADATA globalWsaData;
#endif
	
	SOCKET maxSockNum;
	
	int rc = 0, res = 0, iterations=0;
	long charsread = 0;		
	char buf[BUFLEN];
	
	fd_set tempset, tempset2;
#ifdef MACIGOR
	struct timeval sleeper;
#endif
	vector<SOCKET> openSockets;
	vector<SOCKET>::iterator iter;
	waveBufferInfo *wbi;
	
    for(;;){
		pthread_mutex_lock(&readThreadMutex);
		
//		if(pthread_mutex_trylock( &readThreadMutex ))
//			continue;
			
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
		
		res = select((int) maxSockNum+1, &tempset, 0, 0, &timeout);
		if(res > 0){
			pinstance->getListOfOpenSockets(openSockets);
			for(iter = openSockets.begin() ; iter != openSockets.end() ; iter++) {
				if (FD_ISSET(*iter, &tempset) && pinstance->getWaveBufferInfo(*iter)->toClose == false) {
					wbi = pinstance->getWaveBufferInfo(*iter);
					
					iterations = 0;
					charsread = 0;
					rc = 0;
					//					memset(buf, 0, BUFLEN * sizeof(char));
					do{
						//						memset(buf, 0, BUFLEN * sizeof(char));
						iterations += 1;
						//read the characters from the socket
						rc = recvfrom(*iter, buf, BUFLEN, 0, NULL, NULL);
						charsread += rc;
							
						if (rc <= 0) {
                            wbi->toClose = true;
						} else if(rc > 0)
							wbi->readBuffer.append(buf, rc);

						FD_ZERO(&tempset2);
						timeout.tv_sec = 0;
						timeout.tv_usec =  0;
						FD_SET(*iter, &tempset2);
						res = select((int)(*iter) + 1, &tempset2, 0, 0, &timeout);
						
					} while (res > 0 && rc > 0 && wbi->toClose == false);					
				}
			}
			
/*
			for (ii=0; ii<maxSockNum + 1; ii++) { 
				if (FD_ISSET(ii, &tempset) && pinstance->getWaveBufferInfo(ii)->toClose == false) {
					iters = 0;
					charsread = 0;
					rc = 0;
//					memset(buf, 0, BUFLEN * sizeof(char));
					do{
//						memset(buf, 0, BUFLEN * sizeof(char));
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
*/			
		}
 
		pthread_mutex_unlock( &readThreadMutex );

#ifdef WINIGOR
		Sleep(10);
#endif
#ifdef MACIGOR
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
	snprintf(timestamp, 100 * sizeof(char), "%ld-%ld-%ld_T%02ld:%02ld:%02ld",year, month, day, hour, minute, second );
}

int waveBufferInfo::log_msg(const char *msg, int isSend){
	int err = 0;
	streampos fsize = 0;
	char filename[MAX_FILENAME_LEN + 1];
	char fullfilepath[MAX_PATH_LEN + 1];
	
	long year, month, day, hour, minute, second;
	
	if(!logFile && strlen(logFilePath)){		
		GetTheTime(&year, &month, &day, &hour, &minute, &second);	
		snprintf(filename, MAX_FILENAME_LEN * sizeof(char), "LOG%ld-%ld-%ldT%02ld%02ld%02ld_%d.txt", year, month, day, hour, minute, second, sockNum);
		
		memset(fullfilepath, 0, MAX_PATH_LEN + 1);
		strncpy(fullfilepath, logFilePath, MAX_PATH_LEN);
		strncat(fullfilepath, filename, MAX_PATH_LEN - strlen(fullfilepath));
		
		logFile = new ofstream(fullfilepath, ios_base::app | ios::binary | ios::out);
		if(!logFile){
			logFile = NULL;
			memset(logFilePath, 0, sizeof(char) * (MAX_PATH_LEN + 1));
			err = COULDNT_CREATE_LOGFILE;
			goto done;
		}
		if(logFile && !logFile->is_open()){
			err = COULDNT_CREATE_LOGFILE;
			delete logFile;
			logFile = NULL;
			memset(logFilePath, 0, sizeof(char) * (MAX_PATH_LEN + 1));
			goto done;
		}
		*logFile << hostIP << " " << port << endl;
	}
	
	
	if(strlen(logFilePath) && logFile && logFile->is_open() && strlen(msg) > 0){
		time_t theTime;
		theTime = time(NULL);

		//time to open a new file, max log file size is 50Mb?
		fsize = logFile->tellp();
		if(fsize > 52428800){
			logFile->close();
			GetTheTime(&year, &month, &day, &hour, &minute, &second);	
			snprintf(filename, MAX_FILENAME_LEN * sizeof(char), "LOG%ld-%ld-%ldT%02ld%02ld%02ld_%d.txt", year, month, day, hour, minute, second, sockNum);
			
			memset(fullfilepath, 0, MAX_PATH_LEN + 1);
			strncpy(fullfilepath, logFilePath, MAX_PATH_LEN);
			strncat(fullfilepath, filename, MAX_PATH_LEN - strlen(fullfilepath));
			
			logFile->open(fullfilepath, ios_base::app | ios::binary | ios::out);
			if(!logFile->is_open()){
				delete logFile;
				memset(logFilePath, 0, sizeof(char) * (MAX_PATH_LEN + 1));
				err = COULDNT_CREATE_LOGFILE;
				goto done;
			}
		}
		
		*logFile << theTime << "\t";
		if(isSend)
			*logFile << "SEND" << "\t";
		if(isSend == 0)
			*logFile << "RECV" << "\t";
		
		*logFile << msg << endl;
	} 
done:	
	return err;
}


void CurrentConnections::Instance(){
//	extern CurrentConnections* pinstance;
	
	if(pinstance == 0)
		pinstance = new CurrentConnections(); // create sole instance
}

CurrentConnections::CurrentConnections(){
	FD_ZERO(&readSet);
	maxSockNumber = 0;
	quitReadThreadFlag = false;
	bufferWaves.clear();
	totalSocketsOpened = 0;
	usingProcessor = false;
};

CurrentConnections::~CurrentConnections(){
};


void CurrentConnections::resetCurrentConnections(){
	SOCKET ii;
	//this method resets all the open connections by deleting the waveBufferInfo from 
	//the map of open sockets.  The waveBufferInfo destructor does all the work.
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
	waveBufferInfo *wbi;

	/* Disconnect from server */
	if (FD_ISSET(sockNum, &(readSet))) { 
		FD_CLR(sockNum, &(readSet)); 
		close(sockNum);
		resetMaxSocketNumber();
	}
	wbi = getWaveBufferInfo(sockNum);
	
	wbi->log_msg("CLOSE", -1);
		
	bufferWaves.erase(sockNum);
	
	return err;
}

int CurrentConnections::addWorker(SOCKET sockNum, waveBufferInfo &bufferInfo, waveHndl bufWavH){
	int err = 0;
	waveBufferInfo *wbi;
	
	bufferWaves.insert(make_pair(sockNum, bufferInfo));

	FD_SET(sockNum,&(readSet));
	if(sockNum > maxSockNumber){
		maxSockNumber = sockNum;
	}
	wbi = getWaveBufferInfo(sockNum);
	
	totalSocketsOpened += 1;
    if(bufWavH){
		if(err = HoldWave(bufWavH))
            goto done;
        wbi->bufferWaveRef = bufWavH;
    }
	wbi->log_msg("OPEN", -2);
	
	done:
	return err;
};


int CurrentConnections::checkIfWaveInUseAsBuf(waveHndl wav){
	int inUse = 0;
//	int ii;
	
	vector<SOCKET> openSockets;
	vector<SOCKET>::iterator iter;
	
	if(bufferWaves.empty())
		return 0;
	
	getListOfOpenSockets(openSockets);
	
    for( iter = openSockets.begin(); iter != openSockets.end(); iter++ )
		if(bufferWaves[*iter].bufferWaveRef == wav)
			return 1;
		
//	for (ii=0; ii< maxSockNumber+1 ; ii+=1){
//		if (FD_ISSET(ii, &(readSet))) { 
//			if(bufferWaves[ii].bufferWave == wav)
//				return 1;
//		} 
//	}
	
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
	SOCKET intVal;
	map<SOCKET, waveBufferInfo>::iterator iter;
	
	if(IsNaN64(&query))
		return 0;
	if(IsINF64(&query))
		return 0;
	if(query<=0)
		return 0;
	
	intVal = (SOCKET) query;
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
	
	if(err = GetFunctionInfo(processor, fip))
		err = PROCESSOR_NOT_AVAILABLE;
	
	if(err = CheckFunctionForm(fip, 2, requiredParameterTypes, &badParam, NT_FP64))
		err = PROCESSOR_NOT_AVAILABLE;
	
done:
	return err;
}

int CurrentConnections::checkRecvData(){
	int err = 0;
	
	char report[MAX_MSG_LEN+1];
	vector<SOCKET> openSockets;
	vector<SOCKET>::iterator iter;
	
	if(bufferWaves.empty())
		return err;
	
	getListOfOpenSockets(openSockets);
		
    for( iter = openSockets.begin(); iter != openSockets.end(); iter++ ) {
		if(!bufferWaves[*iter].NOIDLES && bufferWaves[*iter].readBuffer.length() > 0){

			if(err = outputBufferDataToWave(*iter, (const unsigned char*) bufferWaves[*iter].readBuffer.data(),  bufferWaves[*iter].readBuffer.length(), true))
				goto done;
			if(bufferWaves[*iter].toPrint == true){
				memset(report, 0, sizeof(report));
				snprintf(report,sizeof(report),"SOCKITmsg: Socket %d says: \r", *iter);
				XOPNotice(report);
				
				string output;
				output = string(bufferWaves[*iter].readBuffer);
				find_and_replace(output,"\n", "\r");
				XOPNotice(output.c_str());
				XOPNotice("\r");
			}
			bufferWaves[*iter].readBuffer.clear();
			if(bufferWaves[*iter].toClose){
				memset(report, 0, sizeof(report));
				snprintf(report,sizeof(report),"SOCKITmsg: closing socket %d\r", *iter);
				XOPNotice(report);
				pinstance->closeWorker(*iter);
			}
		}
	}
	
done:	
	return err;
}

int CurrentConnections::outputBufferDataToWave(SOCKET sockNum, const unsigned char *writebuffer, size_t szwritebuffer, bool useProcessor){
	int err = 0;
	
	int numDimensions = 2; 
	CountInt dimensionSizes[MAX_DIMENSIONS+1]; 
	CountInt indices[MAX_DIMENSIONS];
	CountInt originalInsertPoint;
	
	Handle wavDataH = NULL;
	
	vector<string> tokens, existingTokens;
	vector<PSInt> tokenSizes;
	vector<string>::iterator tokens_iter, existingTokens_iter;
	stringstream ss;

	size_t szTotalTokens;
	unsigned long numTokens;
	
	CountInt ii;
	
	char report[MAX_MSG_LEN+1];
	
	char timestamp[101];
	
	SOCKITcallProcessorStruct callProcessor;
    FunctionInfo fi;
	waveBufferInfo *wbi = NULL;
    waveHndl wav = NULL;
	
	double result;
	
	wbi = getWaveBufferInfo(sockNum);
	
	if(!wbi){
		err = NO_WAVE_BUFFER_INFO;
		goto done;
	}
	
	wav = wbi->bufferWaveRef;
	
	if(!wav){
		err = NO_WAVE_BUFFER_INFO;
		goto done;
	}

	if(bufferWaves[sockNum].sztokenizer)
		Tokenize(writebuffer, szwritebuffer, tokens, tokenSizes, &szTotalTokens, bufferWaves[sockNum].tokenizer,bufferWaves[sockNum].sztokenizer);
	else {
		tokens.push_back(string((const char*) writebuffer, szwritebuffer));
		szTotalTokens = szwritebuffer;
	}
	numTokens = (unsigned long) tokens.size();
	
	//redimension the text wave to put the tokens in, and put them in.
	// Clear all dimensions sizes to avoid undefined values. 
	MemClear(dimensionSizes, sizeof(dimensionSizes)); 
	MemClear(indices, sizeof(indices)); 
	
	//Get the time stamp
	GetTimeStamp(timestamp);
	
	if (err = MDGetWaveDimensions(wav, &numDimensions, dimensionSizes)) 
		goto done;
    
    if(dimensionSizes[1] != 2){
        dimensionSizes[1] = 2;
        dimensionSizes[2] = 0;
        if(err = MDChangeWave(wav, -1, dimensionSizes))
            goto done;
    }
	
	originalInsertPoint = dimensionSizes[0];
	indices[0] = originalInsertPoint;
    
    //lets get the existing wavedata into a string vector
    if((err = textWaveToTokens(wav, existingTokens)))
       goto done;
    
    //insert the new tokens into the existing tokens, at the end of the first column
    existingTokens.reserve(existingTokens.size() + numTokens * 2);
    existingTokens_iter = existingTokens.begin() + originalInsertPoint;
    existingTokens.insert(existingTokens_iter, tokens.begin(), tokens.end());
  
    //now append the time stamp numtokens times
    for(ii = 0 ; ii < numTokens ; ii++)
        existingTokens.push_back(timestamp);

	dimensionSizes[0] += numTokens;
	dimensionSizes[1] = 2;
	if(err = MDChangeWave(wav, -1, dimensionSizes))
		goto done;
    
    //now put the existing tokens back into the textwave
    if(err = textWaveAccess(&wav, existingTokens))
        goto done;
    
	//make the wave as being modified
	WaveHandleModified(wav);
	
	//call a processor for each buffer entry to see if there's anything it has to do with it.
	if(useProcessor && strlen(wbi->processor)){
		MemClear(indices, sizeof(indices)); 
		indices[1] = 0;
				
		for(ii = originalInsertPoint ; ii < numTokens + originalInsertPoint ; ii++){
			if(checkProcessor(sockNum,  &fi)){
				XOPNotice("SOCKIT error: processor must be f(textWave,variable)\r");
			} else {
				//say that we are calling the processor
				usingProcessor = true;

				callProcessor.entryRow = ii;
				callProcessor.bufferWave = wav;
				if(err = CallFunction(&fi, &callProcessor, &result))
					goto done;
			}
		}
	}
	
	MemClear(dimensionSizes, sizeof(dimensionSizes)); 
	
	if (err = MDGetWaveDimensions(wav, &numDimensions, dimensionSizes))
		goto done;
    
	if(dimensionSizes[0] > BUFFER_WAVE_LEN){
        //lets delete some points
        textWaveToTokens(wav, existingTokens);
        //delete tokens from 2nd column of wave
        existingTokens.erase(existingTokens.begin() + dimensionSizes[0], existingTokens.begin() + (2 * dimensionSizes[0] - BUFFER_TO_KEEP));
        //delete tokens from 1st column of wave
        existingTokens.erase(existingTokens.begin(), existingTokens.begin() + dimensionSizes[0] - BUFFER_TO_KEEP);
        
		dimensionSizes[0] = BUFFER_TO_KEEP;
 		if(err = MDChangeWave2(wav, -1, dimensionSizes, 0))
			goto done;
        if(err = textWaveAccess(&wav, existingTokens))
            goto done;
	}
	
done:
	//we are no longer calling the processor
	usingProcessor = false;
	
	if(err){
		snprintf(report, sizeof(char) * MAX_MSG_LEN, "Oh dear - ERROR NUMBER IS %d: ERRNO is: %d", err, errno);
		XOPNotice(report);
	}

	if(wavDataH)
		WMDisposeHandle(wavDataH);
	
	return err;
}

long CurrentConnections::getTotalSocketsOpened(){
	return totalSocketsOpened;
}

long CurrentConnections::getCurrentSocketsOpened(){
	return (long)bufferWaves.size();
}

void CurrentConnections::getListOfOpenSockets(vector<SOCKET>& list){
	list.clear();
	std::map<SOCKET, waveBufferInfo>::const_iterator iter;
	iter = bufferWaves.begin();
	for(iter ; iter != bufferWaves.end() ; iter++){
		list.push_back(iter->first);
	}
}
