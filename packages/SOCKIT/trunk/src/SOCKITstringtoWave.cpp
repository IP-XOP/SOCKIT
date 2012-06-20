// Operation template: SOCKITstringtoWave/B number:num,string:conv

#include "SOCKITstringtoWave.h"
#include "SwapEndian.h"
#include "defines.h"
#include "StringTokenizer.h"
#include "TextWaveAccess.h"

using namespace std;

static int
ExecuteSOCKITstringtoWave(SOCKITstringtoWaveRuntimeParamsPtr p){
	int err = 0;
	int dataType;
	int bytesPerPoint;
	CountInt numElements;
	int dataFormat, isComplex;
	BCInt szString;
	void *wp;
	
	waveHndl destWaveH;	// Destination wave handle
	int destWaveRefIdentifier;	// Identifies a wave reference
	char destWaveName[MAX_OBJ_NAME+1];
	DataFolderHandle dfH;
	CountInt dimensionSizes[MAX_DIMENSIONS+1];
	int options;
	int destWaveCreated;
	
	size_t szTotalTokens;
	vector<string> tokens;
	vector<PSInt> tokenSizes;
	vector<PSInt>::iterator tokenSizes_iter;
	Handle textDataH = NULL;
	char* delim = NULL;
	char *defaultdelimiter = ";";
	size_t delimeterSize = 1;
	
	
	if(igorVersion < 620 && !RunningInMainThread())
		return NOT_IN_THREADSAFE;
	
	destWaveH = NULL;
	destWaveRefIdentifier = 0;
	dfH = NULL;	// Default is current data folder	
	strcpy(destWaveName, "W_stringToWave"); // Default dest wave name
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
	
	if(p->DESTFlagEncountered){
		strcpy(destWaveName, p->dest.name);
		dfH = p->dest.dfH;
		destWaveRefIdentifier = p->DESTFlagParamsSet[0];
	}
	options = kOpDestWaveOverwriteOK | kOpDestWaveOverwriteExistingWave;
	
	if(p->FREEFlagEncountered && igorVersion < 620)
		return IGOR_OBSOLETE;
	if(p->FREEFlagEncountered)
		options |= kOpDestWaveMakeFreeWave;

	
	szString = GetHandleSize(p->conv);
	dataType = (int)(p->num);

	/* you want to put a string into a textwave, this is going to be done via a tokenizer*/
	if(dataType == 0){
		dataType = TEXT_WAVE_TYPE;
		
		if(p->TOKFlagEncountered && p->TOKFlagStrH){
			delim = *(p->TOKFlagStrH);
			delimeterSize = GetHandleSize(p->TOKFlagStrH);
		} else {
			delim = defaultdelimiter;
			delimeterSize = 1;
		}
				   
		/* now tokenize */
		Tokenize((const unsigned char *) *(p->conv), GetHandleSize(p->conv), tokens, tokenSizes, &szTotalTokens, delim, delimeterSize);
		numElements = tokens.size();
	} else {
		if(err = NumTypeToNumBytesAndFormat(dataType, 
								   &bytesPerPoint,
								   &dataFormat,
								   &isComplex))
			goto done;

		numElements = (CountInt)(szString / bytesPerPoint);
		if(isComplex)
			numElements /= 2;

		if(numElements * bytesPerPoint != szString){
			if(isComplex && numElements * bytesPerPoint * 2 == szString){
				//it was complex, do nothing
			} else {
				err = STRING_INCORRECT_LEN_FOR_NUMTYPE;
				goto done;
			}
			
		}		
	}
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

	
	dimensionSizes[0] = numElements;

	if(err = GetOperationDestWave(dfH, destWaveName, destWaveRefIdentifier,
										options, dimensionSizes, dataType, &destWaveH, &destWaveCreated))
		goto done;
	
	if(numElements == 0)
		goto done;
	
	if(dataType == 0){
		if(err = textWaveAccess(&destWaveH, tokens, tokenSizes, szTotalTokens))
			goto done;
	} else {
		wp = (void*) WaveData(destWaveH);

		//copy over the data.
		memcpy(wp, *(p->conv), GetHandleSize(p->conv));
		
		//E says you expect the data to be big Endian
		//need to byte swap
		if((little_endian == p->EFlagEncountered) || (!little_endian == !p->EFlagEncountered))
			FixByteOrder(wp, bytesPerPoint, numElements);
	}

	if (destWaveRefIdentifier)
	   SetOperationWaveRef(destWaveH, destWaveRefIdentifier);
	
	WaveHandleModified(destWaveH);

done:
	if(textDataH)
		DisposeHandle(textDataH);
	
	return err;
}

int
RegisterSOCKITstringtoWave(void)
{
	char* cmdTemplate;
	char* runtimeNumVarList;
	char* runtimeStrVarList;

	// NOTE: If you change this template, you must change the SOCKITstringtoWaveRuntimeParams structure as well.
	cmdTemplate = "SOCKITstringtoWave/E/DEST=DataFolderAndName:{dest,real}/FREE/TOK=string number:num,string:conv";
	runtimeNumVarList = "";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITstringtoWaveRuntimeParams), (void*)ExecuteSOCKITstringtoWave, kOperationIsThreadSafe);
};

