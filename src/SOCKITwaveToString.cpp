// Operation template: SOCKITstringtoWave/B number:num,string:conv

#include "SOCKITwaveToString.h"
#include "SwapEndian.h"

int
ExecuteSOCKITwaveToString(SOCKITwaveToStringRuntimeParamsPtr p)
{
	int err = 0;
	int waveType;
	long bytesForWave;
	long szString;

	void *wp;
	int hStateWav = 0;

	long numDimensions;
	long dimensionSizes[MAX_DIMENSIONS+1];

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

	if (p->strEncountered) {
		// Parameter: p->str
	}
	
	waveType = WaveType(p->wavWaveH);

	switch(waveType){
		case 2:
			bytesForWave = 4;
		break;
		case 4:
			bytesForWave = 8;
		break;
		case 8:
			bytesForWave = 1;
		break;
		case 16:
			bytesForWave = 2;
		break;
		case 32:
			bytesForWave = 4;
		break;
		case 96:
			bytesForWave = 4;
		break;
		case 80:
			bytesForWave = 2;
			break;
		case 72:
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
	cmdTemplate = "SOCKITwaveToString/E wave:wav, varname:str";
	runtimeNumVarList = "";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITwaveToStringRuntimeParams), (void*)ExecuteSOCKITwaveToString, 0);
}
