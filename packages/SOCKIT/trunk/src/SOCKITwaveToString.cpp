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
	int dataType;
	int bytesPerPoint, dataFormat, isComplex;
	size_t szString;
	long numElements;
	void *wp;
	Handle textDataP = NULL;
	
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

	numElements = WavePoints(p->wavWaveH);
	dataType = WaveType(p->wavWaveH);
	
	if(dataType == TEXT_WAVE_TYPE){
		//IT'S A TEXT WAVE
		long *pTableOffset;
		char *pTextDataStart, *pTextDataEnd;
		
		if(err = GetTextWaveData(p->wavWaveH, 2, &textDataP))
			goto done;
		
		pTableOffset = (long*)*textDataP;					// Pointer to table of offsets if mode==1 or mode==2
		
		pTextDataStart = *textDataP;
		pTextDataStart += (numElements + 1) * sizeof(long);
		pTextDataEnd = *textDataP;
		pTextDataEnd +=  pTableOffset[numElements];
		szString = pTextDataEnd - pTextDataStart;
			
		if(err = StoreStringDataUsingVarName(p->str, pTextDataStart, szString))
			goto done;
		
	} else {
		//IT'S A NUMERICAL WAVE
		//if the TXT flag is not specified you get a binary representation of the wave
		//this is useful for sending arrays over networks, compressing arrays, etc.
		if(!p->TXTFlagEncountered){
			int little_endian = IsLittleEndian();
			
			if(err = NumTypeToNumBytesAndFormat(dataType, 
												&bytesPerPoint,
												&dataFormat,
												&isComplex))
				goto done;
			
			if(isComplex)
				bytesPerPoint *= 2;
			
			szString = bytesPerPoint * numElements;
			
			wp = (void*)WaveData(p->wavWaveH);
			
			//E says you expect the data to be big Endian
			//need to byte swap
			if((little_endian == p->EFlagEncountered) || (!little_endian == !p->EFlagEncountered))
				FixByteOrder(wp, bytesPerPoint, numElements);
			
			if(	err = StoreStringDataUsingVarName(p->str, (char*)wp, szString))
				goto done;
			
		} else {
			//TXT flag makes a text representation of the wave
			long ii;
			int val;
			wp = (void*) WaveData(p->wavWaveH);
			
			for(ii=0 ; ii < numElements ; ii+=1){
				switch(dataType){
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
				if(chunk.append(tempNumStr, sizeof(char), strlen(tempNumStr)) == -1){
					err = NOMEM;
					goto done;
				}
			}		
			if(err = StoreStringDataUsingVarName(p->str, (char*)chunk.getData(), chunk.getMemSize()))
				goto done;
		}
	}
done:
	if (textDataP)
		DisposeHandle(textDataP);
	
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
