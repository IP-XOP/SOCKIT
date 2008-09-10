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

int GetTheTime(long *year, long *month, long *day, long *hour, long *minute, long *second){
	time_t rawtime;
	tm *theTime ;
	
	time(&rawtime);
	theTime = localtime(&rawtime);
	*year = -100+theTime->tm_year;
	*month = 1+theTime->tm_mon;
	*day = theTime->tm_mday;
	*hour = theTime->tm_hour;
	*minute = theTime->tm_min;
	*second = theTime->tm_sec; 
	return 0;
	
}


CurrentConnections* pinstance=0;

void CurrentConnections::Instance(){
	extern CurrentConnections* pinstance;
	
	if(pinstance==0){
		pinstance = new CurrentConnections(); // create sole instance
	}
}

CurrentConnections::CurrentConnections(){
	FD_ZERO(&readSet);
	maxSockNumber = 0;
	bufferWaves.clear();
};

CurrentConnections::~CurrentConnections(){
};


void CurrentConnections::resetCurrentConnections(){
	SOCKET ii;
	for (ii=0; ii< maxSockNumber+1 ; ii+=1){
		if (FD_ISSET(ii, &(readSet))) { 
			bufferWaves[ii].bufferWave =NULL;
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

void CurrentConnections::resetMaxSocketNumber(){
	
	SOCKET ii=0;
	SOCKET maxSoFar=0;
	
	for (ii=0; ii< maxSockNumber+1 ; ii+=1){
		if (FD_ISSET(ii, &(readSet)))
			maxSoFar = ii;
	}
	maxSockNumber = maxSoFar;
}

const waveBufferInfo* CurrentConnections::getWaveBufferInfo(SOCKET sockNum){
	return &(bufferWaves[sockNum]);
};


int CurrentConnections::closeWorker(SOCKET sockNum){
	int err = 0;
	/* Disconnect from server */
	if (FD_ISSET(sockNum, &(readSet))) { 
		FD_CLR(sockNum, &(readSet)); 
		close(sockNum);
		resetMaxSocketNumber();
		
/*		if(getWaveBufferInfo(sockNum)->logDoc != NULL ){
			rewind((pinstance->getWaveBufferInfo(sockNum)->logFile));
			if(xmlDocFormatDump((pinstance->getWaveBufferInfo(sockNum)->logFile),pinstance->getWaveBufferInfo(sockNum)->logDoc,0)==-1){
				XOPCloseFile((pinstance->getWaveBufferInfo(sockNum)->logFile));
			}
			
			xmlFreeDoc(getWaveBufferInfo(sockNum)->logDoc);
			XOPCloseFile((pinstance->getWaveBufferInfo(sockNum)->logFile));
		}
*/		
		//shut down the buffering
		//remove the memory
		bufferWaves[sockNum].~waveBufferInfo();
		bufferWaves.erase(sockNum);
		err = 0;
	} else {
		err = -1;
	}
	
	return err;
}

void CurrentConnections::addWorker(SOCKET sockNum,waveBufferInfo bufferInfoStruct){
	
	bufferWaves[sockNum] = bufferInfoStruct;
	
	FD_SET(sockNum,&(readSet));
	if(sockNum > maxSockNumber){
		maxSockNumber = sockNum;
	}
};


int CurrentConnections::checkIfWaveInUseAsBuf(waveHndl wav){
	int inUse = 0;
	int ii;
	
	waveBufferInfo bufferStruct;
	
	for (ii=0; ii< maxSockNumber+1 ; ii+=1){
		if (FD_ISSET(ii, &(readSet))) { 
			bufferStruct = bufferWaves[ii];
			if(bufferStruct.bufferWave == wav)
				return 1;
		} 
	}
	
	return inUse;
}


int CurrentConnections::registerProcessor(SOCKET sockNum, const char *processor){
	int err=0;
	
	FunctionInfo fi;
    
	memset(bufferWaves[sockNum].processor,0,sizeof(bufferWaves[sockNum].processor));
	strlcpy(bufferWaves[sockNum].processor,processor,sizeof(bufferWaves[sockNum].processor));
	
	if(strlen(processor)==0){
		goto done;
	}
	
	if(err = GetFunctionInfo(processor,&fi)){
		err = PROCESSOR_NOT_AVAILABLE;
	}
	
	if(err = checkProcessor(sockNum, &fi)){
		err = PROCESSOR_NOT_AVAILABLE;
    }
	
done:
	return err;
}

int CurrentConnections::isSockitOpen(double query,SOCKET *sockNum){
	int retval=0;
	*sockNum = (SOCKET)-1;
	double roundedVal;
	long intVal;
	
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
	
	if(FD_ISSET(intVal,pinstance->getReadSet())){
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
	
	if(err = GetFunctionInfo(processor,fip)){
		err = PROCESSOR_NOT_AVAILABLE;
	}
	
	if(err = CheckFunctionForm(fip, 2, requiredParameterTypes, &badParam,NT_FP64)){
		err = PROCESSOR_NOT_AVAILABLE;
	}
	
done:
	return err;
}

int CurrentConnections::checkRecvData(){
	int err = 0;
	
#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif
	
	SOCKET maxSockNum = maxSockNumber;
	SOCKET ii;
	
	char buf[BUFLEN+1];
	
	int rc = 0, res = 0;
	int iters =0;
	long charsread = 0;
	
	MemoryStruct chunk;
	
	char report[MAX_MSG_LEN+1];
	
	fd_set tempset;
	FD_ZERO(&tempset);
	
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
	memcpy(&tempset, &readSet, sizeof(readSet)); 
	res = select(maxSockNum+1,&tempset,0,0,&timeout);
	if(res == 0)
		goto done;
	
	if(res == -1 && maxSockNum > 0){
		XOPNotice("SOCKIT err: problem with select()\r");
		goto done;
	}
	
	for (ii=0; ii<maxSockNum+1; ii++) { 
		iters = 0;
		charsread = 0;
		
		if (FD_ISSET(ii, &tempset)) {
			chunk.reset();	//clear the memory buffers so it can be reused
			
			do{
				iters += 1;
				//read the characters from the socket
				rc = recv(ii, buf, BUFLEN,0);
				charsread += rc;
				
				if (rc <= 0 && iters == 1) {
					if(bufferWaves[ii].toPrint){
						snprintf(report,sizeof(report),"SOCKIT err: socket descriptor %d, disconnection???\r", ii );
						XOPNotice(report);
					}
					// Closed connection or error 
					if(pinstance->closeWorker(ii))
						XOPNotice("SOCKIT error: SOCKIT tried to remove socket that wasn't open\r");
					
					break;
				} else if(rc > 0){
					chunk.WriteMemoryCallback(buf, sizeof(char), rc);
					if(chunk.getData() == NULL){
						err = NOMEM;
						goto done;
					}
				}
			}while (rc==BUFLEN);
			
			if(charsread>0){
				if(err = outputBufferDataToWave(ii, chunk.getData(), chunk.getMemSize()))
					goto done;
				if(bufferWaves[ii].toPrint == true){
					snprintf(report,sizeof(report),"SOCKITmsg: Socket %d says: \r", ii);
					XOPNotice(report);
					
					string output;
					output = string(chunk.getData(),chunk.getMemSize());
					find_and_replace(output,"\n","\r");
					XOPNotice(output.c_str());
					XOPNotice("\r");
				}
			}
			
			timeout.tv_sec = 0.;
		}
	}
	
done:
	FD_ZERO(&tempset);
	
	return err;
}

int CurrentConnections::outputBufferDataToWave(SOCKET sockNum, const char *writebuffer, size_t szwritebuffer){
	int err = 0;
	
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
	
	long year,month,day,hour,minute,second;
	char timebuf[100];
	
	SOCKITcallProcessorStruct callProcessor;
    FunctionInfo fi;
    waveHndl wav = bufferWaves[sockNum].bufferWave;
	
	//do you want to debug?
	bool DBUG = bufferWaves[sockNum].DBUG;
	
	//if you want a logfile
	xmlNode *added_node = NULL;
	xmlNode *root_element = NULL;
	xmlChar *encContent = NULL;
	
	double result;
	
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
		
		GetTheTime(&year,&month,&day,&hour,&minute,&second);
		snprintf(timebuf, 99, "%02ld/%02ld/%02ld %02ld:%02ld:%02ld",year,month,day,hour,minute,second);
		
		if(err = PutCStringInHandle(timebuf,textH))
			goto done;
		indices[1] = 1;
		
		if(err = MDSetTextWavePointValue(wav,indices,textH))
			goto done;
		
		//to Debug put the socket number in the 3rd column
		if(DBUG){
			snprintf(timebuf,sizeof(timebuf),"%d",sockNum);
			if(err = PutCStringInHandle(timebuf,textH))
				goto done;
			indices[1] = 2;
			if(err = MDSetTextWavePointValue(wav,indices,textH))
				goto done;
		}
		
		if(dimensionSizes[0] > BUFFER_WAVE_LEN){
			pointsToDelete = 300;//dimensionSizes[0] - BUFFER_WAVE_LEN;
			indices[0] -= pointsToDelete;
			
			if(err = GetWavesDataFolder(wav,&dfH))
				goto done;
			if(err = GetDataFolderNameOrPath(dfH,1,pathName))
				goto done;
			WaveName(bufferWaves[sockNum].bufferWave,waveName);
			strlcat(pathName,waveName,sizeof(pathName));
			strlcpy(cmd,"Deletepoints 0,",sizeof(cmd));
			snprintf(pointsToDeleteStr,sizeof(pointsToDeleteStr),"%d,",pointsToDelete);
			strlcat(cmd,pointsToDeleteStr,sizeof(cmd));
			strlcat(cmd,pathName,sizeof(cmd));
			if(err = XOPSilentCommand(cmd))
				goto done;
		}
		
		//if there is a logfile then append and save
		if(pinstance->getWaveBufferInfo(sockNum)->logDoc != NULL){
			root_element = xmlDocGetRootElement(pinstance->getWaveBufferInfo(sockNum)->logDoc);
			encContent = xmlEncodeEntitiesReentrant(pinstance->getWaveBufferInfo(sockNum)->logDoc, BAD_CAST tokens.at(ii).c_str());
			added_node = xmlNewChild(root_element, NULL, BAD_CAST "RECV" ,encContent);
			if(encContent != NULL){
				xmlFree(encContent);
				encContent = NULL;
			}
			encContent = xmlEncodeEntitiesReentrant(pinstance->getWaveBufferInfo(sockNum)->logDoc, BAD_CAST timebuf);
			xmlSetProp(added_node, BAD_CAST "time", encContent);
			
			rewind(pinstance->getWaveBufferInfo(sockNum)->logFile);
			if(xmlDocFormatDump((pinstance->getWaveBufferInfo(sockNum)->logFile),pinstance->getWaveBufferInfo(sockNum)->logDoc,0)==-1){
				XOPCloseFile((pinstance->getWaveBufferInfo(sockNum)->logFile));
			}
		}
		
		//call a processor for each buffer entry to see if there's anything it has to do with it.
		if(checkProcessor(sockNum,  &fi)){
			XOPNotice("SOCKIT error: processor must be f(textWave,variable)\r");
		} else {
			if(strlen(getWaveBufferInfo(sockNum)->processor)==0)
				continue;
			callProcessor.entryRow = indices[0];
			callProcessor.bufferWave = wav;
			if(err = CallFunction(&fi,&callProcessor,&result))
				goto done;
		}
		
		WaveHandleModified(wav);
	}
done:
	if(textH!=NULL)
		DisposeHandle(textH);
	if(encContent != NULL)
		xmlFree(encContent);
	
	return err;
}


