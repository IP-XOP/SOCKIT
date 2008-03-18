#include "SOCKIT.h"

int registerProcessor(SOCKET sockNum, const char *processor){
	int err=0;
	
	extern currentConnections openConnections;
	FunctionInfo fip;
		
	strlcpy(openConnections.bufferWaves[sockNum].processor,processor,MAX_OBJ_NAME);
	openConnections.bufferWaves[sockNum].processorfip = NULL;
	
	if(strlen(processor)==0){
		openConnections.bufferWaves[sockNum].processorfip = NULL;	
		goto done;
	}
	
	if(err = GetFunctionInfo(processor,&fip)){
		XOPNotice("SOCKITprocessor requires two parameters, f(textwave,variable)\r");
		err = PROCESSOR_NOT_AVAILABLE;
	}
	
	if(err = checkProcessor(processor, &fip)){
		openConnections.bufferWaves[sockNum].processorfip = NULL;
		err = PROCESSOR_NOT_AVAILABLE;
	} else {
		openConnections.bufferWaves[sockNum].processorfip = &fip;
	}
	
done:
		
		return err;
}

int deRegisterProcessor(SOCKET sockNum){
	int err=0;
	extern currentConnections openConnections;
	
	openConnections.bufferWaves[sockNum].processorfip = NULL;
	
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
	
	if(err = GetCStringFromHandle(p->processor,processor,MAX_OBJ_NAME))
		goto done;
		
	if(err = registerProcessor(p->sockNum, processor))
		goto done;
	
done:
	if(p->processor)
		DisposeHandle(p->processor);

return err;
}