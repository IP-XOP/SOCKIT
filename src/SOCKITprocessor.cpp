#include "SOCKIT.h"

int registerProcessor(SOCKET sockNum, const char *processor){
	int err=0;
	
	extern currentConnections openConnections;
	FunctionInfo fi;
    
	memset(openConnections.bufferWaves[sockNum].processor,0,MAX_OBJ_NAME);
	strlcpy(openConnections.bufferWaves[sockNum].processor,processor,MAX_OBJ_NAME);
	
	if(strlen(processor)==0){
		goto done;
	}
	
	if(err = GetFunctionInfo(processor,&fi)){
		XOPNotice("SOCKITprocessor requires two parameters, f(textwave,variable)\r");
		err = PROCESSOR_NOT_AVAILABLE;
	}
	
	if(err = checkProcessor(processor, &fi)){
		err = PROCESSOR_NOT_AVAILABLE;
    }
	
done:
		
		return err;
}

int deRegisterProcessor(SOCKET sockNum){
	int err=0;
	extern currentConnections openConnections;
	
	memset(openConnections.bufferWaves[sockNum].processor,0,MAX_OBJ_NAME);
	
done:
		return err;
}

int checkProcessor(const char *processor, FunctionInfo *fip){
	int err=0;
	
	int requiredParameterTypes[2];
	
	requiredParameterTypes[0] = TEXT_WAVE_TYPE;
	requiredParameterTypes[1] = NT_FP64;
	int badParam = 0;
	
	if(strlen(processor)==0){
		err = PROCESSOR_NOT_AVAILABLE;
		goto done;
	}
	
/**	if(fip == NULL){
		err = PROCESSOR_NOT_AVAILABLE;
		goto done;
	}
	*/
	if(err = GetFunctionInfo(processor,fip)){
		XOPNotice("SOCKITprocessor requires two parameters, f(textwave,variable)\r");
		err = PROCESSOR_NOT_AVAILABLE;
	}
	
	if(err = CheckFunctionForm(fip, 2, requiredParameterTypes, &badParam,NT_FP64)){
		XOPNotice("SOCKITprocessor requires two parameters, f(textwave,variable)\r");
		err = PROCESSOR_NOT_AVAILABLE;
	}
	
done:
		return err;
}

int SOCKITregisterProcessor(SOCKITprocessorStruct *p){
	int err = 0;
	char processor[MAX_OBJ_NAME+1];
	
	extern currentConnections openConnections;
	SOCKET sockNum;
	
    p->retval = 0;
    
	fd_set tempset;
	FD_ZERO(&tempset);
	memcpy(&tempset, &openConnections.readSet, sizeof(openConnections.readSet)); 
	
	sockNum = (SOCKET)p->sockNum;
			
	if(!FD_ISSET(sockNum,&tempset)){
			err = NO_SOCKET_DESCRIPTOR;
			goto done;
	}
	
	if(err = GetCStringFromHandle(p->processor,processor,MAX_OBJ_NAME))
		goto done;
		
	if(err = registerProcessor(p->sockNum, processor))
		goto done;
	
done:
	if(p->processor)
		DisposeHandle(p->processor);
    if(err)
        p->retval = -1;
        
return err;
}