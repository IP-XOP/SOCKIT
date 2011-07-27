// Operation template: SOCKITstringtoWave/B number:num,string:conv

#include "SOCKITstringtoWave.h"
#include "SwapEndian.h"
#include "defines.h"

static int
ExecuteSOCKITstringtoWave(SOCKITstringtoWaveRuntimeParamsPtr p)
{
	int err = 0;
	int dataType;
	int bytesPerPoint;
	long numElements;
	int dataFormat, isComplex;

	unsigned long szString;
	void *wp;

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
			goto done;
		}
	}
	
	szString = GetHandleSize(p->conv);
	dataType = (int)(p->num);

	if(err = NumTypeToNumBytesAndFormat(dataType, 
							   &bytesPerPoint,
							   &dataFormat,
							   &isComplex))
		goto done;
	
	
/*	switch(waveType){
		case 2:	//NT_FP32
			bytesPerPoint = 4;
			break;
		case 3:	//NT_FP32 | NT_CMPLX
			bytesPerPoint = 8;
			break;
		case 4:	//NT_FP64
			bytesPerPoint = 8;
			break;
		case 5:	//NT_FP64 | NT_CMPLX
			bytesPerPoint = 16;
			break;
		case 8:	//NT_I8
			bytesPerPoint = 1;
			break;
		case 9:	//NT_I8 | NT_CMPLX
			bytesPerPoint = 2;
			break;
		case 16:  //NT_I16
			bytesPerPoint = 2;
			break;
		case 17:  //NT_I16 | NT_CMPLX
			bytesPerPoint = 4;
			break;			
		case 32:  //NT_I32
			bytesPerPoint = 4;
			break;
		case 33:  //NT_I32 | NT_CMPLX
			bytesPerPoint = 8;
			break;			
		case 96:  //NT_I32 | NT_UNSIGNED
			bytesPerPoint = 4;
			break;
		case 97: //NT_I32 | NT_UNSIGNED | NT_CMPLX
			bytesPerPoint = 8;
			break;			
		case 80:  //NT_I16 | NT_UNSIGNED
			bytesPerPoint = 2;
			break;
		case 81:  //NT_I16 | NT_UNSIGNED | NT_CMPLX
			bytesPerPoint = 4;
			break;
		case 72: //NT_I8 | NT_UNSIGNED
			bytesPerPoint = 1;
			break;
		case 73: //NT_I8 | NT_UNSIGNED | NT_CMPLX
			bytesPerPoint = 2;
			break;
			
		default:	//not recognized wavetype
			goto done;
			break;
	}*/

	numElements = szString / bytesPerPoint;
	if(isComplex)
		numElements *= 2;
	
	if(numElements * bytesPerPoint != szString){
		err = STRING_INCORRECT_LEN_FOR_NUMTYPE;
		goto done;
	}
	
	dimensionSizes[0] = numElements;
	
	strcpy(waveName, "W_stringToWave");
	if(err = MDMakeWave(&waveH, waveName, NULL, dimensionSizes, dataType, 1))
		goto done;

	wp = (void*) WaveData(waveH);

	//copy over the data.
	memcpy(wp, *(p->conv), GetHandleSize(p->conv));
	
	//E says you expect the data to be big Endian
	//need to byte swap
	if((little_endian == p->EFlagEncountered) || (!little_endian == !p->EFlagEncountered))
		FixByteOrder(wp, bytesPerPoint, numElements);
					
	WaveHandleModified(waveH);

done:

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

