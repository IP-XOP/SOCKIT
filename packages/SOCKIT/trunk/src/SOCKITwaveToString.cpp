// Operation template: SOCKITstringtoWave/B number:num,string:conv

#include "SOCKITwaveToString.h"
#include "SwapEndian.h"
#include "memutils.h"
#include <map>

#define NUMCHARS 50
#ifdef _WINDOWS_
#define snprintf sprintf_s
#endif

using namespace std;


int
ExecuteSOCKITwaveToString(SOCKITwaveToStringRuntimeParamsPtr p)
{
	int err = 0;
	int waveType;
	long bytesForWave;
	long szString;
	
	int hStateWav = 0;
	long numDimensions;
	long dimensionSizes[MAX_DIMENSIONS+1];
	void *wp;
	
	MemoryStruct chunk;
	register char tempNumStr[NUMCHARS+1];
	
	// Main parameters.
	if (p->wavEncountered) {
		// Parameter: p->wav (test for NULL handle before using)
		if(!p->wavWaveH){
			err = EXPECTED_NUMERIC_WAVE;
			goto done;
		}
	}
	
	if (p->strEncountered) {
		if (p->strParamsSet[0]) {
			// Optional parameter: p->ret
			int dataTypePtr;
			if(err = VarNameToDataType(p->str, &dataTypePtr))
				goto done;
			if(dataTypePtr){
				err = OH_EXPECTED_STRING;
				goto done;
			}
		}
	}
	
	//if the TXT flag is not specified you get a binary representation of the wave
	//this is useful for sending arrays over networks, compressing arrays, etc.
	if(!p->TXTFlagEncountered){
		waveType = WaveType(p->wavWaveH);
		
		switch(waveType){
			case NT_FP32:
				bytesForWave = 4;
				break;
			case NT_FP64:
				bytesForWave = 8;
				break;
			case NT_I8:
				bytesForWave = 1;
				break;
			case NT_I16:
				bytesForWave = 2;
				break;
			case NT_I32:
				bytesForWave = 4;
				break;
			case (NT_UNSIGNED|NT_I32):
				bytesForWave = 4;
				break;
			case (NT_UNSIGNED|NT_I16):
				bytesForWave = 2;
				break;
			case NT_UNSIGNED|NT_I8:
				bytesForWave = 1;
				break;
			default:	//not recognized wavetype
				goto done;
				break;
		}
		
		szString = bytesForWave * WavePoints(p->wavWaveH);
		
		//E says you expect the data to be big Endian
		//need to byte swap
		if(IsLittleEndian() == p->EFlagEncountered){
			if(err = MDGetWaveDimensions(p->wavWaveH,&numDimensions,dimensionSizes))
				goto done;
			
			if(err = MDChangeWave2(p->wavWaveH,-1, dimensionSizes, 2))
				goto done;
		}
		
		hStateWav = MoveLockHandle(p->wavWaveH);
		wp = (void*)WaveData(p->wavWaveH);
		
		if(	err = StoreStringDataUsingVarName(p->str, (char*)wp, szString))
			goto done;
		
		HSetState(p->wavWaveH, hStateWav);
		
		//E says you expect the data to be big Endian
		//need to byte swap
		if(IsLittleEndian() == p->EFlagEncountered){
			if(err = MDChangeWave2(p->wavWaveH,-1, dimensionSizes, 2))
				goto done;		
		}
	} 
	
	//TXT flag makes a text representation of the wave
	if(p->TXTFlagEncountered){
		long ii, totalIts;
		int val;
		waveType = WaveType(p->wavWaveH);
		wp = (void*) WaveData(p->wavWaveH);
		totalIts = WavePoints(p->wavWaveH);
		hStateWav = MoveLockHandle(p->wavWaveH);
		
		for(ii=0 ; ii< totalIts ; ii+=1){
			switch(waveType){
				case NT_FP32:
					val = snprintf(tempNumStr, NUMCHARS, "%g ", ((float *)wp)[ii]);
					break;
				case NT_FP64:
					val = snprintf(tempNumStr, NUMCHARS,"%g ", ((double *)wp)[ii]);
					break;
				case NT_I8:
					val = snprintf(tempNumStr, NUMCHARS, "%hhd ", ((char* )wp)[ii]);
					break;
				case NT_I16:
					val = snprintf(tempNumStr, NUMCHARS,"%hd ", ((short* )wp)[ii]);
					break;
				case NT_I32:
					val = snprintf(tempNumStr, NUMCHARS,"%li ", ((long* )wp)[ii]);
					break;
				case (NT_UNSIGNED|NT_I32):
					val = snprintf(tempNumStr, NUMCHARS,"%d ", ((unsigned long* )wp)[ii]);
					break;
				case (NT_UNSIGNED|NT_I16):
					val = snprintf(tempNumStr, NUMCHARS, "%d ", ((unsigned short* )wp)[ii]);
					break;
				case NT_UNSIGNED|NT_I8:
					val = snprintf(tempNumStr, NUMCHARS, "%d ", ((unsigned char* )wp)[ii]);
					break;
				default:	//not recognized wavetype
					goto done;
					break;
			}
			try {
				chunk.WriteMemoryCallback(tempNumStr, sizeof(char), strlen(tempNumStr));
			} catch (bad_alloc&){
				HSetState(p->wavWaveH, hStateWav);
				err = NOMEM;
				goto done;
			}
		}		
		HSetState(p->wavWaveH, hStateWav);
		if(err = StoreStringDataUsingVarName(p->str, (char*)chunk.getData(), chunk.getMemSize()))
			goto done;
	}
done:
	
	return err;
}

int
RegisterSOCKITwaveToString(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the SOCKITwaveToStringRuntimeParams structure as well.
	cmdTemplate = "SOCKITwaveToString/E/TXT wave:wav, varname:str";
	runtimeNumVarList = "";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITwaveToStringRuntimeParams), (void*)ExecuteSOCKITwaveToString, 0);
}
