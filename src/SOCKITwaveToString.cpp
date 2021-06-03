/*
 *  SOCKITwaveToString.cpp
 *  SOCKIT
 *
 *  Created by andrew on 25/04/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
// Operation template: SOCKITstringtoWave/B number:num,string:conv

#include "SOCKITwaveToString.h"
#include "SwapEndian.h"
#include <map>
#include <string>
#include <iostream>
#include <sstream>

#define NUMCHARS 50
#ifdef WINIGOR
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
	CountInt numElements;
	void *wp;
	Handle textDataP = NULL;
	
	string chunk;
	
	// Main parameters.
	if (p->wavEncountered) {
		// Parameter: p->wav (test for NULL handle before using)
		if(!p->wavWaveH){
			err = EXPECTED_NUMERIC_WAVE;
			goto done;
		}
	} else {
        err = EXPECTED_WAVE_REF;
        goto done;
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
    if(!p->wavWaveH){
        err = EXPECTED_WAVE_REF;
        goto done;
    }

	numElements = WavePoints(p->wavWaveH);
	dataType = WaveType(p->wavWaveH);
	
	if(dataType == TEXT_WAVE_TYPE){
		//IT'S A TEXT WAVE
		IndexInt *pTableOffset;
		char *pTextDataStart, *pTextDataEnd;

		if(err = GetTextWaveData(p->wavWaveH, 2, &textDataP))
			goto done;

		pTableOffset = (IndexInt*)*textDataP;					// Pointer to table of offsets for mode==2
		
		if(p->TXTFlagEncountered && p->TXTFlagParamsSet[0] && p->TXTFlagStrH){
			string listsep;
			string output;
			listsep = string(*(p->TXTFlagStrH), WMGetHandleSize(p->TXTFlagStrH));

			for(CountInt ii = 0 ; ii < numElements ; ii++){
				output.append(*textDataP + pTableOffset[ii], pTableOffset[ii + 1] - pTableOffset[ii]);
				output.append(listsep);
			}
			if(err = StoreStringDataUsingVarName(p->str, output.data(), output.length()))
				goto done;
		} else {
			pTextDataStart = *textDataP;
			pTextDataStart += (numElements + 1) * sizeof(long);
			pTextDataEnd = *textDataP;
			pTextDataEnd +=  pTableOffset[numElements];
			szString = pTextDataEnd - pTextDataStart;
			
			if(err = StoreStringDataUsingVarName(p->str, pTextDataStart, szString))
				goto done;
		}
		
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
			CountInt ii;
			stringstream oss;
			string listsep = string(" ");
			if(p->TXTFlagParamsSet[0] && p->TXTFlagStrH)
				listsep = string(*(p->TXTFlagStrH), WMGetHandleSize(p->TXTFlagStrH));
			
			wp = (void*) WaveData(p->wavWaveH);
			
			for(ii=0 ; ii < numElements ; ii+=1){
				switch(dataType){
					case NT_FP32:
						oss << ((float*)wp)[ii] << listsep;
						break;
					case NT_FP64:
						oss << ((double*)wp)[ii] << listsep;
						break;
					case NT_I8:
						oss << (int)((char*)wp)[ii] << listsep;
						break;
					case NT_I16:
						oss << ((short*)wp)[ii] << listsep;
						break;
					case NT_I32:
						oss << ((SInt32*)wp)[ii] << listsep;
						break;
                    case NT_I64:
                        oss << ((SInt64*)wp)[ii] << listsep;
                        break;
                    case (NT_UNSIGNED|NT_I64):
                        oss << ((UInt64*)wp)[ii]<< listsep;
                        break;
                    case (NT_UNSIGNED|NT_I32):
						oss << ((UInt32*)wp)[ii]<< listsep;
						break;
					case (NT_UNSIGNED|NT_I16):
						oss << ((unsigned short*)wp)[ii]<< listsep;
						break;
					case NT_UNSIGNED|NT_I8:
						oss << (int)((unsigned char*)wp)[ii]<< listsep;
						break;
					default:	//not recognized wavetype
						goto done;
						break;
				}
			}		
			chunk = oss.str();
			if(err = StoreStringDataUsingVarName(p->str, (char*)chunk.data(), chunk.length()))
				goto done;
		}
	}
done:
	if (textDataP)
		WMDisposeHandle(textDataP);
	
	return err;
}

int
RegisterSOCKITwaveToString(void)
{
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the SOCKITwaveToStringRuntimeParams structure as well.
	cmdTemplate = "SOCKITwaveToString/E/TXT[=string] wave:wav, varname:str";
	runtimeNumVarList = "";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SOCKITwaveToStringRuntimeParams), (void*)ExecuteSOCKITwaveToString, kOperationIsThreadSafe);
}
