 /*
 *  TextWaveAccess.cpp
 *  SOCKIT
 *
 *  Created by andrew on 20/06/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */
#include "TextWaveAccess.h"

#include <vector>
#include <sstream>
#include <iterator>
#include <string>

using namespace std;

int textWaveAccess(waveHndl *textWaveH, vector<string> &tokens){
	int err = 0;
	Handle textDataH = NULL;
	IndexInt *pTableOffset = 0;
	char *pTextData;
	CountInt ii = 0;
    size_t siz0, siz1, siz2;
	stringstream ss;
    size_t szTotalTokens = 0;
	vector<string>::iterator tokens_iter;
	
	if(!*textWaveH){
		err = NOWAV;
		goto done;
	}
	if(WaveType(*textWaveH)){
		err = EXPECTED_TEXT_WAVE;
		goto done;
	}
	
	//this is just a check to stop IGOR crashing.
	//the caller of this function must ensure that the number of wavepoints is the same as 
	if(WavePoints(*textWaveH) != tokens.size()){
		err = PNTS_INCOMPATIBLE;
		goto done;
	}
	
	if(err = GetTextWaveData(*textWaveH, 2, &textDataH))
		goto done;

    std::copy(tokens.begin(), tokens.end(), ostream_iterator<string>(ss));
    szTotalTokens = ss.str().length();
	
	//resize the handle
	SetHandleSize(textDataH, szTotalTokens + (tokens.size() + 1) * sizeof(PSInt));
	if(err = MemError())
		goto done;
	
	//point to table of offsets
	pTableOffset = (PSInt*)*textDataH;                                 

	//now set the offsets
	for(tokens_iter = tokens.begin(), ii = 0 ; tokens_iter != tokens.end() ; tokens_iter++, ii++){
		pTableOffset[ii + 1] = pTableOffset[ii] + (*tokens_iter).size();
        siz0 = (*tokens_iter).size();
        siz1 = pTableOffset[ii];
        siz2 = pTableOffset[ii + 1];
    }
	
    pTextData = *textDataH + (tokens.size() + 1) * sizeof(PSInt);
	memcpy(pTextData, ss.str().data(), szTotalTokens);
    
	if(err = SetTextWaveData(*textWaveH, 2, textDataH))
		goto done;
	
done:
	
	if(textDataH)
		DisposeHandle(textDataH);
	return err;
}

int textWaveToTokens(waveHndl *textWaveH, vector<string> &tokens){
	int err = 0;
    Handle textDataH = NULL;
	IndexInt *pTableOffset;
	char *pTextData;
	CountInt ii;
    CountInt numpnts = 0;
    string tempStr;
	
	if(!*textWaveH){
		err = NOWAV;
		goto done;
	}
	if(WaveType(*textWaveH)){
		err = EXPECTED_TEXT_WAVE;
		goto done;
	}

    numpnts = WavePoints(*textWaveH);
    tokens.clear();
    tokens.reserve(numpnts);

	if(err = GetTextWaveData(*textWaveH, 2, &textDataH))
		goto done;
		
	//point to table of offsets
	pTableOffset = (PSInt*)*textDataH;
    pTextData = *textDataH + (numpnts + 1) * sizeof(PSInt);
    
	//now set the offsets
	for(ii = 0; ii < numpnts; ii++){
        tokens.push_back(string(pTextData, pTableOffset[ii + 1] - pTableOffset[ii]));
        pTextData += pTableOffset[ii + 1] - pTableOffset[ii];
    }
		
done:
    if(textDataH)
		DisposeHandle(textDataH);
	return err;
}

