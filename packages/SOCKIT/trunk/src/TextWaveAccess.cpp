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

int textWaveAccess(waveHndl *textWaveH, vector<string> &tokens, vector<PSInt> tokenSizes, PSInt szTotalTokens){
	int err = 0;
	Handle textDataH;
	IndexInt *pTableOffset;
	char *pTextData;
	CountInt ii;
	stringstream ss;
	vector<PSInt>::iterator tokenSizes_iter;
	
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
	
	//resize the handle
	SetHandleSize(textDataH, szTotalTokens + (tokens.size() + 1) * sizeof(PSInt));
	if(err = MemError())
		goto done;
	
	//point to table of offsets
	pTableOffset = (PSInt*)*textDataH;                                 
	pTextData = *textDataH + (tokens.size() + 1) * sizeof(PSInt);
	
	std::copy(tokens.begin(), tokens.end(), ostream_iterator<string>(ss));
	
	memcpy(pTextData, ss.str().data(), szTotalTokens);
	//now set the offsets
	for(tokenSizes_iter = tokenSizes.begin(), ii = 0 ; tokenSizes_iter != tokenSizes.end() ; tokenSizes_iter++, ii++)
		pTableOffset[ii + 1] = pTableOffset[ii] + *tokenSizes_iter;
	
	if(err = SetTextWaveData(*textWaveH, 2, textDataH))
		goto done;
	
done:
	
	if(textDataH)
		DisposeHandle(textDataH);
	return err;
}

