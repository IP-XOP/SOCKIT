#include "SOCKIT.h"
#include "SOCKITsendmsg.h"

int
RegisterSOCKITsendmsg(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the SOCKITsendmsgRuntimeParams structure as well.
	cmdTemplate = "SOCKITsendmsg number:ID,string:MSG";
	runtimeNumVarList = "V_Flag";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITsendmsgRuntimeParams), (void*)ExecuteSOCKITsendmsg, 0);
}

int 
ExecuteSOCKITsendmsg(SOCKITsendmsgRuntimeParams *p){
	int err = 0, err2 = 0;
	
	extern CurrentConnections *pinstance;
	
#ifdef _WINDOWS_
	extern WSADATA globalWsaData;
#endif
	
    int rc = 0;
    SOCKET socketToWrite = -1;
	int res = 0;
    
	char buf[BUFLEN+1];
	char report[MAX_MSG_LEN+1];
	char *output = NULL;			//get rid of the carriage returns
	
	xmlNode *added_node = NULL;
	xmlNode *root_element = NULL;
	xmlChar *encContent = NULL;
	long year,month,day,hour,minute,second;
	char timebuf[100];
	
	
	SOCKET maxSockNum = pinstance->getMaxSockNumber();
	
	fd_set tempset;
	FD_ZERO(&tempset);
	
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
    
	memset(buf,0,sizeof(buf));
	memcpy(&tempset, pinstance->getReadSet(), sizeof(*(pinstance->getReadSet()))); 
	
	if(!p->MSGEncountered){
		err = OH_EXPECTED_STRING;
		goto done;
	} else {
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
	} else {
		if(!FD_ISSET(socketToWrite,&tempset)){
			snprintf(report,sizeof(report),"SOCKIT err: can't write to socket %d\r", socketToWrite);
			XOPNotice(report);
			err2 = 1;
			goto done;
		}
	}
	
	//flush messages first
	err = pinstance->checkRecvData();
	
	res = select(maxSockNum+1,0,&tempset,0,&timeout);
	if(res == -1){
		if(pinstance->getWaveBufferInfo(socketToWrite)->toPrint == true)
			XOPNotice ("SOCKIT err: select returned -1");
		err2 = 1;
        goto done;
	}
	if(FD_ISSET(socketToWrite,&tempset)){
		rc = send(socketToWrite,buf,strlen(buf),0);
		if(rc >= 0){
			snprintf(report,sizeof(report),"SOCKITmsg: wrote to socket %d\r", socketToWrite);
			output = NtoCR(buf, "\n","\r");
			if( pinstance->getWaveBufferInfo(socketToWrite)->toPrint == true){
				XOPNotice(report);
				XOPNotice(output);
				XOPNotice("\r");
			}			
			//if there is a logfile then append and save
			if(pinstance->getWaveBufferInfo(socketToWrite)->logDoc != NULL){
				root_element = xmlDocGetRootElement(pinstance->getWaveBufferInfo(socketToWrite)->logDoc);
				encContent = xmlEncodeEntitiesReentrant(pinstance->getWaveBufferInfo(socketToWrite)->logDoc, BAD_CAST output);
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
	if(output!= NULL)
		free(output);
	
	if(encContent != NULL)
		xmlFree(encContent);
	
	if(err || err2){
		SetOperationNumVar("V_flag", 1);
	} else {
		SetOperationNumVar("V_flag", 0);
	}
	
	FD_ZERO(&tempset);
	
	return err;
}