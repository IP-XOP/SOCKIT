/*
 *  TextWaveAccess.h
 *  SOCKIT
 *
 *  Created by andrew on 20/06/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "XOPStandardHeaders.h"
#include <vector>
#include <string>
using namespace std;

//User must make sure that the number of wavepoints (however they are distributed between dimensions)
//must be equal to the number of tokens. Otherwise an error will be raised.
int textWaveAccess(waveHndl *textWaveH, vector<string> &tokens, vector<PSInt> tokenSizes, PSInt szTotalTokens);
