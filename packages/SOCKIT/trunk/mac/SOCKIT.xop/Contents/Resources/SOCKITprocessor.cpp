#include "SOCKIT.h"

int registerProcessor(long sockNum, char processor[MAX_OBJ_NAME+1]){
	int err=0;
	
	extern currentConnections openConnections;
	FunctionInfo *fip;
	
	openConnections.bufferWaves[sockNum].processor = processor;
	
	if(strlen(processor)==0){
		openConnections.bufferWaves[sockNum].processorfip = NULL;
		goto done;
	}
	
	if(err = checkProcessor(processor, fip)){
		openConnections.bufferWaves[sockNum].processorfip = NULL;
		err = 0;
	} else {
		openConnections.bufferWaves[sockNum].processorfip = *fip;
	}
	
done:
		
		return err;
}

int deRegisterProcessor(long sockNum){
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
		err = 0;
		goto done;
	}
	
	if(err = GetFunctionInfo(processor,fip)){
		XOPNotice("SOCKITprocessor requires two parameters, f(textwave,variable)\r");
		err = 1;
	}
	
	if(err = CheckFunctionForm(fip, 2, requiredParameterTypes, &badParam,NT_FP64)){
		XOPNotice("SOCKITprocessor requires two parameters, f(textwave,variable)\r");
		err = 1;
	}
	
done:
		return err;
}