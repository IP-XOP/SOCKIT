#include "SOCKIT.h"
#include "SOCKITsendmsg.h"

int
RegisterSOCKITsendmsg(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the SOCKITsendmsgRuntimeParams structure as well.
	cmdTemplate = "SOCKITsendmsg/TIME=number number:ID,string:MSG";
	runtimeNumVarList = "V_Flag";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITsendmsgRuntimeParams), (void*)ExecuteSOCKITsendmsg, 0);
}

int 
ExecuteSOCKITsendmsg(SOCKITsendmsgRuntimeParams *p){
	int err = 0, err2 = 0;
	
	extern CurrentConnections *pinstance;
	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );
			
#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif
	
    int rc = 0;
    SOCKET socketToWrite = -1;
	int res = 0;
    
	char buf[BUFLEN+1];
	char report[MAX_MSG_LEN+1];
	string output;			//get rid of the carriage returns
	
	xmlNode *added_node = NULL;
	xmlNode *root_element = NULL;
	xmlChar *encContent = NULL;
	long year,month,day,hour,minute,second;
	char timebuf[100];
	long size = 0;
	fd_set tempset;
	struct timeval timeout;
	
	memset(buf,0,sizeof(buf));
	
	if(p->TIMEFlagEncountered){
		timeout.tv_sec = floor(p->TIMEFlagNumber);
		timeout.tv_usec =  (int)(p->TIMEFlagNumber-(double)floor(p->TIMEFlagNumber))*1000000;
	} else {
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
    }
	
	if(!p->MSGEncountered){
		err = OH_EXPECTED_STRING;
		goto done;
	} else {
		size = GetHandleSize(p->MSG);
		if(size>BUFLEN){
			err2 = 1;
			XOPNotice("SOCKIT err: message is longer than buffer\r");
			goto done;
		}
		if(err = GetCStringFromHandle(p->MSG, buf, sizeof(buf)))
			goto done;
	}
	
	if(!p->IDEncountered){
		err = OH_EXPECTED_NUMBER;
		goto done;
	}
	
	if(!pinstance->isSockitOpen(p->ID,&socketToWrite)){
		snprintf(report,sizeof(report),"SOCKIT err: socket not connected %d\r", socketToWrite);
		XOPNotice(report);
		err2 = 1;
		goto done;
	}
		
	FD_ZERO(&tempset);
	FD_SET(socketToWrite, &tempset);
	
	res = select(socketToWrite+1, 0, &tempset, 0, &timeout);
	if(res == -1){
		if(pinstance->getWaveBufferInfo(socketToWrite)->toPrint == true)
			XOPNotice ("SOCKIT err: select returned -1");
		err2 = 1;
        goto done;
	}
	if(FD_ISSET(socketToWrite,&tempset)){
		rc = send(socketToWrite, buf, GetHandleSize(p->MSG), 0);
		if(rc >= 0){
			snprintf(report,sizeof(report), "SOCKITmsg: wrote to socket %d\r", socketToWrite);
			output = string(buf, GetHandleSize(p->MSG));
			find_and_replace(output, "\n", "\r");
			
			if( pinstance->getWaveBufferInfo(socketToWrite)->toPrint == true){
				XOPNotice(report);
				XOPNotice(output.c_str());
				XOPNotice("\r");
			}			
			//if there is a logfile then append and save
			if(pinstance->getWaveBufferInfo(socketToWrite)->logDoc != NULL){
				root_element = xmlDocGetRootElement(pinstance->getWaveBufferInfo(socketToWrite)->logDoc);
				encContent = xmlEncodeEntitiesReentrant(pinstance->getWaveBufferInfo(socketToWrite)->logDoc, BAD_CAST output.c_str());
				added_node = xmlNewChild(root_element, NULL, BAD_CAST "SEND" ,encContent);
				if(encContent != NULL){
					xmlFree(encContent);
					encContent = NULL;
				}
				
				GetTheTime(&year,&month,&day,&hour,&minute,&second);
				snprintf(timebuf, 99, "%02ld/%02ld/%02ld %02ld:%02ld:%02ld",year,month,day,hour,minute,second);
				
				encContent = xmlEncodeEntitiesReentrant(pinstance->getWaveBufferInfo(socketToWrite)->logDoc, BAD_CAST timebuf);
				xmlSetProp(added_node, BAD_CAST "time", encContent);
				
				rewind((pinstance->getWaveBufferInfo(socketToWrite)->logFile));
				if(xmlDocFormatDump((pinstance->getWaveBufferInfo(socketToWrite)->logFile),pinstance->getWaveBufferInfo(socketToWrite)->logDoc,0)==-1){
					XOPCloseFile((pinstance->getWaveBufferInfo(socketToWrite)->logFile));
				}
			}
			goto done;
		} else if (rc < 0) {// Closed connection or error
			snprintf(report,sizeof(report),"SOCKIT err: problem writing to socket descriptor %d, disconnecting\r", socketToWrite );
			if(pinstance->getWaveBufferInfo(socketToWrite)->toPrint == true)
				XOPNotice(report);
			pinstance->closeWorker(socketToWrite);
			err2 = 1;
		}
	} else {
			snprintf(report,sizeof(report),"SOCKIT err: timeout writing to socket %d\r", socketToWrite);
			if(pinstance->getWaveBufferInfo(socketToWrite)->toPrint == true)
				XOPNotice(report);
		err2 = 1;
		goto done;
	}
	
done:
	if(encContent != NULL)
		xmlFree(encContent);
	
	if(err || err2){
		SetOperationNumVar("V_flag", 1);
	} else {
		SetOperationNumVar("V_flag", 0);
	}
	
	FD_ZERO(&tempset);

	pthread_mutex_unlock( &readThreadMutex );

	return err;
}

int 
SOCKITsendmsgF(SOCKITsendmsgFStruct *p){
	int err = 0, err2 = 0;
	
	extern CurrentConnections *pinstance;
	extern pthread_mutex_t readThreadMutex;
	pthread_mutex_lock( &readThreadMutex );

#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif
	
    int rc = 0;
    SOCKET socketToWrite = -1;
	int res = 0;
    
	char buf[BUFLEN+1];
	string output;			//get rid of the carriage returns
	
	xmlNode *added_node = NULL;
	xmlNode *root_element = NULL;
	xmlChar *encContent = NULL;
	long year,month,day,hour,minute,second;
	char timebuf[100];
	long size = 0;
	fd_set tempset;
	struct timeval timeout;
	
	memset(buf,0,sizeof(buf));

	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	
	if(!p->message){
		err = OH_EXPECTED_STRING;
		goto done;
	} else {
		size = GetHandleSize(p->message);
		if(size>BUFLEN){
			err2 = 1;
			goto done;
		}
		if(err = GetCStringFromHandle(p->message, buf, sizeof(buf)))
			goto done;
	}
	
	if(!p->sockID){
		err = OH_EXPECTED_NUMBER;
		goto done;
	}
	
	if(!pinstance->isSockitOpen(p->sockID,&socketToWrite)){
		err2 = 1;
		goto done;
	}
		
	FD_ZERO(&tempset);
	FD_SET(socketToWrite, &tempset);
	
	res = select(socketToWrite+1, 0, &tempset, 0, &timeout);
	if(res == -1){
		err2 = 1;
        goto done;
	}
	if(FD_ISSET(socketToWrite,&tempset)){
		rc = send(socketToWrite, buf, GetHandleSize(p->message), 0);
		if(rc >= 0){
			//if there is a logfile then append and save
			if(pinstance->getWaveBufferInfo(socketToWrite)->logDoc != NULL){
				root_element = xmlDocGetRootElement(pinstance->getWaveBufferInfo(socketToWrite)->logDoc);
				encContent = xmlEncodeEntitiesReentrant(pinstance->getWaveBufferInfo(socketToWrite)->logDoc, BAD_CAST output.c_str());
				added_node = xmlNewChild(root_element, NULL, BAD_CAST "SEND" ,encContent);
				if(encContent != NULL){
					xmlFree(encContent);
					encContent = NULL;
				}
				
				GetTheTime(&year,&month,&day,&hour,&minute,&second);
				snprintf(timebuf, 99, "%02ld/%02ld/%02ld %02ld:%02ld:%02ld",year,month,day,hour,minute,second);
				
				encContent = xmlEncodeEntitiesReentrant(pinstance->getWaveBufferInfo(socketToWrite)->logDoc, BAD_CAST timebuf);
				xmlSetProp(added_node, BAD_CAST "time", encContent);
				
				rewind((pinstance->getWaveBufferInfo(socketToWrite)->logFile));
				if(xmlDocFormatDump((pinstance->getWaveBufferInfo(socketToWrite)->logFile),pinstance->getWaveBufferInfo(socketToWrite)->logDoc,0)==-1){
					XOPCloseFile((pinstance->getWaveBufferInfo(socketToWrite)->logFile));
				}
			}
			goto done;
		} else if (rc < 0) {// Closed connection or error
			pinstance->closeWorker(socketToWrite);
			err2 = 1;
		}
	} else {
		err2 = 1;
		goto done;
	}
	
done:
	if(encContent != NULL)
		xmlFree(encContent);
	
	if(err || err2){
		p->retval = 1;
	} else {
		p->retval = 0;
	}
	
	FD_ZERO(&tempset);
	
	if(p->message)
		DisposeHandle(p->message);
					
	pthread_mutex_unlock( &readThreadMutex );

	return err;
}
