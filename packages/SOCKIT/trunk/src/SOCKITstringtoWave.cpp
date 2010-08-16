// Operation template: SOCKITstringtoWave/B number:num,string:conv

#include "SOCKITstringtoWave.h"
#include "SwapEndian.h"

static int
ExecuteSOCKITstringtoWave(SOCKITstringtoWaveRuntimeParamsPtr p)
{
	int err = 0;
	int waveType;
	long bytesForWave;
	long numElements;
	size_t szString;
	void *wp;
	char *cp = NULL;

	waveHndl waveH;
	char waveName[MAX_OBJ_NAME+1];
	long dimensionSizes[MAX_DIMENSIONS+1];

	memset(dimensionSizes, 0, sizeof(dimensionSizes));

	int little_endian = IsLittleEndian();
	
	// Flag parameters.
	if (p->EFlagEncountered) {
	}

	// Main parameters.

	if (p->numEncountered) {
		// Parameter: p->num
	}

	if (p->convEncountered) {
		// Parameter: p->conv (test for NULL handle before using)
		if(!p->conv){
			err = OH_EXPECTED_STRING;
		}
	}
	
	szString = GetHandleSize(p->conv);

	waveType = (int)(p->num);
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

	numElements = szString / bytesForWave;
	if(numElements * bytesForWave != szString){
		err = 1;
		goto done;
	}
	dimensionSizes[0] = numElements;
	
	strcpy(waveName, "W_stringToWave");
	if(err = MDMakeWave(&waveH, waveName, NULL, dimensionSizes, waveType, 1))
		goto done;

	wp = (void*) WaveData(waveH);

	//copy over the data.  Hmm, a bit scary doing this.
	memcpy(wp, *(p->conv), GetHandleSize(p->conv));
	
	//E says you expect the data to be big Endian
	//need to byte swap
	if((little_endian == p->EFlagEncountered) || (!little_endian == !p->EFlagEncountered)){
		for (register int i = 0; i < numElements; i++) 
			ByteSwap(((unsigned char*) wp) + i, bytesForWave);
		//		if(err = MDChangeWave2(waveH, WaveType(waveH), dimensionSizes, 2))
		//			goto done;
		//		WaveHandleModified(waveH);
	}
	
	WaveHandleModified(waveH);

done:
	if(cp!= NULL)
		free(cp);

	return err;
}

int
RegisterSOCKITstringtoWave(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the SOCKITstringtoWaveRuntimeParams structure as well.
	cmdTemplate = "SOCKITstringtoWave/E number:num,string:conv";
	runtimeNumVarList = "";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITstringtoWaveRuntimeParams), (void*)ExecuteSOCKITstringtoWave, 0);
};

